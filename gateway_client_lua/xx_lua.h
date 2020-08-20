#pragma once

// xx::Lua::Context 系列函数应该被确保调用于 pcall 环境, 方能正确响应 luaL_error
// 经测试发现，默认情况下 luajit 支持 c++ exception, 能析构, lua 5.3 默认不支持, 必须强制以 C++ 方式自己编译
// 另：听说 luajit 在 push 的时候不需要 checkstack
// todo: 自己编译支持 C++ exception 的最新 lua 5.x 源代码 集成到项目, 主要供给服务器端使用

#include "xx_data_rw.h"
#include "xx_string.h"
#include <lua.hpp>

namespace xx {
    /******************************************************************************************************************/
    // Lua 简单数据结构 序列化适配( 不支持 table 循环引用, 注意 lua5.1 不支持 int64, 整数需在 int32 范围内 )
    template<>
    struct DataFuncs<lua_State *, void> {
        // lua 可序列化的数据类型列表( 同时也是对应的 typeId ). 不被支持的类型将忽略
        enum class LuaTypes : uint8_t {
            Nil, True, False, Integer, Double, String, Table, TableEnd
        };

        // 外界需确保栈非空.
        static inline void Write(DataWriter &dw, lua_State *const &in) {
            switch (auto t = lua_type(in, -1)) {
                case LUA_TNIL:
                    dw.Write(LuaTypes::Nil);
                    return;
                case LUA_TBOOLEAN:
                    dw.Write(lua_toboolean(in, -1) ? LuaTypes::True : LuaTypes::False);
                    return;
                case LUA_TNUMBER: {
#if defined(LUA_VERSION_NUM) && LUA_VERSION_NUM >= 530
                    if (lua_isinteger(in, -1)) {
                        dw.Write(LuaTypes::Integer, (int64_t) lua_tointeger(in, -1));
                    } else {
                        dw.Write(LuaTypes::Double);
                        dw.WriteFixed(lua_tonumber(in, -1));
                    }
#else
                    auto d = lua_tonumber(in, -1);
                    auto i = (int64_t) d;
                    if ((double) i == d) {
                        dw.Write(LuaTypes::Integer, i);
                    } else {
                        dw.Write(LuaTypes::Double);
                        dw.WriteFixed(d);
                    }
#endif
                    return;
                }
                case LUA_TSTRING: {
                    dw.Write(LuaTypes::String);
                    size_t len;
                    auto ptr = lua_tolstring(in, -1, &len);
                    dw.Write(std::string_view(ptr, len));
                    return;
                }
                case LUA_TTABLE: {
                    dw.Write(LuaTypes::Table);
                    int idx = lua_gettop(in);                                   // 存 idx 备用
                    lua_checkstack(in, 1);
                    lua_pushnil(in);                                            //                      ... t, nil
                    while (lua_next(in, idx) != 0) {                            //                      ... t, k, v
                        // 先检查下 k, v 是否为不可序列化类型. 如果是就 next
                        t = lua_type(in, -1);
                        if (t == LUA_TLIGHTUSERDATA || t == LUA_TFUNCTION || t == LUA_TUSERDATA || t == LUA_TTHREAD) {
                            lua_pop(in, 1);                                     //                      ... t, k
                            continue;
                        }
                        t = lua_type(in, -2);
                        if (t == LUA_TLIGHTUSERDATA || t == LUA_TFUNCTION || t == LUA_TUSERDATA || t == LUA_TTHREAD) {
                            lua_pop(in, 1);                                     //                      ... t, k
                            continue;
                        }
                        Write(dw, in);                                          // 先写 v
                        lua_pop(in, 1);                                         //                      ... t, k
                        Write(dw, in);                                          // 再写 k
                    }
                    dw.Write(LuaTypes::TableEnd);
                    return;
                }
                default:
                    dw.Write(LuaTypes::Nil);
                    return;
            }
        }

        // 如果读失败, 可能会有残留数据已经压入，外界需自己做 lua state 的 cleanup
        static inline int Read(DataReader &dr, lua_State *&out) {
            LuaTypes lt;
            if (int r = dr.Read(lt)) return r;
            switch (lt) {
                case LuaTypes::Nil:
                    lua_checkstack(out, 1);
                    lua_pushnil(out);
                    return 0;
                case LuaTypes::True:
                    lua_checkstack(out, 1);
                    lua_pushboolean(out, 1);
                    return 0;
                case LuaTypes::False:
                    lua_checkstack(out, 1);
                    lua_pushboolean(out, 0);
                    return 0;
                case LuaTypes::Integer: {
                    int64_t i;
                    if (int r = dr.Read(i)) return r;
                    lua_checkstack(out, 1);
                    lua_pushinteger(out, (lua_Integer) i);
                    return 0;
                }
                case LuaTypes::Double: {
                    lua_Number d;
                    if (int r = dr.ReadFixed(d)) return r;
                    lua_checkstack(out, 1);
                    lua_pushnumber(out, d);
                    return 0;
                }
                case LuaTypes::String: {
                    size_t len;
                    if (int r = dr.Read(len)) return r;
                    lua_checkstack(out, 1);
                    lua_pushlstring(out, dr.buf + dr.offset, len);
                    dr.offset += len;
                    return 0;
                }
                case LuaTypes::Table: {
                    lua_checkstack(out, 4);
                    lua_newtable(out);                                          // ... t
                    while (dr.offset < dr.len) {
                        if (dr.buf[dr.offset] == (char) LuaTypes::TableEnd) {
                            ++dr.offset;
                            return 0;
                        }
                        if (int r = Read(dr, out)) return r;                    // ... t, v
                        if (int r = Read(dr, out)) return r;                    // ... t, v, k
                        lua_pushvalue(out, -2);                                 // ... t, v, k, v
                        lua_rawset(out, -4);                                    // ... t, v
                        lua_pop(out, 1);                                        // ... t
                    }
                }
                default:
                    return __LINE__;
            }
        }
    };
}

namespace xx::Lua {

    /******************************************************************************************************************/
    // Lua push, to 系列基础适配模板. Push 返回实际入栈的参数个数( 通常是 1. 但如果传入一个队列并展开压栈则不一定 ). To 无返回值.
    // 可能抛 lua 异常( 故这些函数应该间接被 pcall 使用 )
    template<typename T, typename ENABLED = void>
    struct PushToFuncs {
        static inline int Push(lua_State *const &L, T const &in) {
            return luaL_error(L, "Lua Push error! not support's type %s", typeid(T).name());
        }

        static inline void To(lua_State *const &L, int const &idx, T &out) {
            luaL_error(L, "Lua Push error! not support's type %s", typeid(T).name());
        }
    };

    // 适配 bool
    template<>
    struct PushToFuncs<bool, void> {
        static inline int Push(lua_State *const &L, bool const &in) {
            lua_checkstack(L, 1);
            lua_pushboolean(L, in ? 1 : 0);
            return 1;
        }

        static inline void To(lua_State *const &L, int const &idx, bool &out) {
            if (!lua_isboolean(L, idx)) luaL_error(L, "error! args[%d] is not bool", idx);
            out = (bool) lua_toboolean(L, idx);
        }
    };

    // 适配 整数
    template<typename T>
    struct PushToFuncs<T, std::enable_if_t<std::is_integral_v<T>>> {
        static inline int Push(lua_State *const &L, T const &in) {
            lua_checkstack(L, 1);
            lua_pushinteger(L, in);
            return 1;
        }

        static inline void To(lua_State *const &L, int const &idx, T &out) {
            int isnum = 0;
            out = (T) lua_tointegerx(L, idx, &isnum);
            if (!isnum) luaL_error(L, "error! args[%d] is not number", idx);
        }
    };

    // 适配 枚举( 转为整数 )
    template<typename T>
    struct PushToFuncs<T, std::enable_if_t<std::is_enum_v<T>>> {
        typedef std::underlying_type_t<T> UT;

        static inline int Push(lua_State *const &L, T const &in) {
            return PushToFuncs<UT, void>::Push(L, (UT) in);
        }

        static inline void To(lua_State *const &L, int const &idx, T &out) {
            PushToFuncs<UT, void>::To(L, (UT &) out);
        }
    };

    // 适配 浮点
    template<typename T>
    struct PushToFuncs<T, std::enable_if_t<std::is_floating_point_v<T>>> {
        static inline int Push(lua_State *const &L, T const &in) {
            lua_checkstack(L, 1);
            lua_pushnumber(L, in);
            return 1;
        }

        static inline void To(lua_State *const &L, int const &idx, T &out) {
            int isnum = 0;
            out = (T) lua_tonumberx(L, idx, &isnum);
            if (!isnum) luaL_error(L, "error! args[%d] is not number", idx);
        }
    };

    // 适配 literal char[len] string
    template<size_t len>
    struct PushToFuncs<char[len], void> {
        static inline int Push(lua_State *const &L, char const(&in)[len]) {
            lua_checkstack(L, 1);
            lua_pushlstring(L, in, len - 1);
            return 1;
        }
    };

    // 适配 std::string
    template<>
    struct PushToFuncs<std::string, void> {
        static inline int Push(lua_State *const &L, std::string const &in) {
            lua_checkstack(L, 1);
            lua_pushlstring(L, in.data(), in.size());
            return 1;
        }

        static inline void To(lua_State *const &L, int const &idx, std::string &out) {
            if (!lua_isstring(L, idx)) luaL_error(L, "error! args[%d] is not string", idx);
            size_t len = 0;
            auto ptr = lua_tolstring(L, idx, &len);
            out.assign(ptr, len);
        }
    };

    // 适配 std::string_view
    template<>
    struct PushToFuncs<std::string_view, void> {
        static inline int Push(lua_State *const &L, std::string_view const &in) {
            lua_checkstack(L, 1);
            lua_pushlstring(L, in.data(), in.size());
            return 1;
        }
    };

    // 适配 char*, char const*
    template<typename T>
    struct PushToFuncs<T, std::enable_if_t<std::is_same_v<T, char *> || std::is_same_v<T, char const *>>> {
        static inline int Push(lua_State *const &L, T const &in) {
            lua_checkstack(L, 1);
            lua_pushstring(L, in);
            return 1;
        }
    };

    // 适配模板转为函数
    namespace Detail {
        template<typename Arg, typename...Args>
        int Push(lua_State *const &L, Arg const &arg) {
            return ::xx::Lua::PushToFuncs<Arg>::Push(L, arg);
        }

        template<typename Arg, typename...Args>
        int Push(lua_State *const &L, Arg const &arg, Args const &...args) {
            int n = ::xx::Lua::PushToFuncs<Arg>::Push(L, arg);
            return n + Push(L, args...);
        }
    }

    template<typename...Args>
    int Push(lua_State *const &L, Args const &...args) {
        return ::xx::Lua::Detail::Push(L, args...);
    }

    // 适配模板转为函数
    namespace Detail {

        template<typename Arg, typename...Args>
        void To(lua_State *const &L, int const &idx, Arg &arg, Args &...args) {
            xx::Lua::PushToFuncs<Arg>::To(L, idx, arg);
            if constexpr(sizeof...(Args)) {
                To(L, idx + 1, args...);
            }
        }

        void To(lua_State *const &L, int const &idx) {
        }
    }

    template<typename...Args>
    void To(lua_State *const &L, int const &idx, Args &...args) {
        xx::Lua::Detail::To(L, idx, args...);
    }

    // 适配 std::tuple
    template<typename...Args>
    struct PushToFuncs<std::tuple<Args...>, void> {
        static inline int Push(lua_State *const &L, std::tuple<Args...> const &in) {
            int rtv = 0;
            std::apply([&](auto const &... args) {
                rtv = Push(L, args...);
            }, in);
            return rtv;
        }

        static inline void To(lua_State *const &L, int const &idx, std::tuple<Args...> &out) {
            std::apply([&](auto &... args) {
                xx::Lua::To(L, idx, args...);
            }, out);
        }
    };


    /******************************************************************************************************************/
    // lambda / function 类型分析

    template<typename T, typename = void>
    struct FuncTraits;

    template<typename Rtv, typename...Args>
    struct FuncTraits<Rtv (*)(Args const &...)> {
        using R = Rtv;
        using A = std::tuple<Args...>;
    };

    template<typename Rtv, typename CT, typename... Args>
    struct FuncTraits<Rtv (CT::*)(Args const &...) const> {
        using R = Rtv;
        using A = std::tuple<Args...>;
    };

    template<typename T>
    struct FuncTraits<T, std::void_t<decltype(&T::operator())> >
            : public FuncTraits<decltype(&T::operator())> {
    };

    template<typename T>
    using FuncR_t = typename FuncTraits<T>::R;
    template<typename T>
    using FuncA_t = typename FuncTraits<T>::A;


    /******************************************************************************************************************/
    // Exec / PCall 返回值封装, 易于使用
    struct Result {
        int n = 0;
        std::string m;

        Result() = default;

        Result(Result const &) = default;

        Result &operator=(Result const &) = default;

        Result(Result &&o) : n(std::exchange(o.n, 0)), m(std::move(o.m)) {}

        Result &operator=(Result &&o) {
            std::swap(n, o.n);
            std::swap(m, o.m);
            return *this;
        }

        inline operator bool() const {
            return n != 0;
        }
    };


    /******************************************************************************************************************/
    // Lua 简单封装 为方便易用
    struct Context {
        lua_State *L = nullptr;
        bool needClose = false;

        explicit Context(bool const &openLibs = true) {
            L = luaL_newstate();
            if (!L) throw std::logic_error("auto &&L = luaL_newstate(); if (!L)");
            if (openLibs) {
                luaL_openlibs(L);
            }
            needClose = true;
        }

        explicit Context(lua_State *L) : L(L), needClose(false) {}

        Context(Context const &) = delete;

        Context &operator=(Context const &) = delete;

        Context(Context &&o) noexcept: L(o.L), needClose(o.needClose) {
            o.L = nullptr;
            o.needClose = false;
        }

        Context &operator=(Context &&o) noexcept {
            std::swap(L, o.L);
            std::swap(needClose, o.needClose);
            return *this;
        }

        ~Context() {
            if (L && needClose) {
                lua_close(L);
            }
        }

        // 方便使用
        inline operator lua_State *() {
            return L;
        }

        // luaL_error
        template<typename...Args>
        inline int Error(Args const &... args) {
            return luaL_error(L, ::xx::ToString(args...).c_str());
        }

        // 安全执行 lambda。不负责还原 top. 如果中途有 luaL_error 则 返回错误码 和 错误文本。理论上讲所有 lua 调用都该包这个
        template<typename T>
        Result Try(T &&func) {
            Result rtv;
            if (!lua_checkstack(L, 2)) {
                rtv.n = -1;
                rtv.m = "lua_checkstack(L, 1) failed. not enough memory??";
                return rtv;
            }
            lua_pushlightuserdata(L, &func);                                // ..., &func
            lua_pushcclosure(L, [](auto L) {                                // ..., cfunc
                // 从 upvalue 区间取出 userdata, 析构
                auto &&f = (T *) lua_touserdata(L, lua_upvalueindex(1));
                (*f)();
                return 0;
            }, 1);
            if ((rtv.n = lua_pcall(L, 0, LUA_MULTRET, 0))) {    // ...
                rtv.m = lua_tostring(L, -1);
            }
            return rtv;
        }

        // [-0, +0, -]
        inline int GetTop() {
            return lua_gettop(L);
        }

        // [-?, +?, -]
        inline void SetTop(int const &index) {
            lua_settop(L, index);
        }

        // [-n, +0, -]
        inline void Pop(int const &n) {
            lua_pop(L, n);
        }

        inline void AssertTop(int const &n, char const *const &msg = "") {
            auto top = lua_gettop(L);
            if (top != n) luaL_error(L, "AssertTop( %d ) failed! current top = %d%s", n, top, msg);
        }

        // [-0, +1, e]
        inline void GetGlobal(char const *const &key) {
            lua_getglobal(L, key);
        }

        inline void GetGlobal(std::string const &key) {
            lua_getglobal(L, key.c_str());
        }

        // [-1, +0, e]
        inline void SetGlobal(char const *const &key) {
            lua_setglobal(L, key);
        }

        inline void SetGlobal(std::string const &key) {
            lua_setglobal(L, key.c_str());
        }

        template<typename T>
        inline void SetGlobalBy(char const *const &key, T const &v) {
            Push(v);
            lua_setglobal(L, key);
        }

        template<typename T>
        inline void SetGlobalBy(std::string const &key, T const &v) {
            Push(v);
            lua_setglobal(L, key.c_str());
        }


        template<typename T>
        inline void GetGlobalTo(char const *const &key, T &v) {
            auto top = lua_gettop(L);
            lua_getglobal(L, key);                  // ..., v
            To(v);
            lua_settop(L, top);
        }

        template<typename T>
        inline void GetGlobalTo(std::string const &key, T &v) {
            auto top = lua_gettop(L);
            lua_getglobal(L, key.c_str());          // ..., v
            To(v);
            lua_settop(L, top);
        }


        // [-0, +?, m]
        inline void DoFile(char const *const &fileName) {
            luaL_dofile(L, fileName);
        }

        inline void DoFile(std::string const &fileName) {
            luaL_dofile(L, fileName.c_str());
        }

        template<typename...Args>
        int Push(Args const &...args) {
            return ::xx::Lua::Push(L, args...);
        }

        int Push() { return 0; }

        template<typename...Args>
        void To(Args &...args) {
            return ::xx::Lua::To(L, 1, args...);
        }

        void To() {}

        // [-n, +1, m]
        // 先压 up values 再压 cfunc
        template<typename...Args>
        inline void PushFunc(lua_CFunction const &func, Args const &...args) {
            lua_checkstack(L, 1);
            lua_pushcclosure(L, func, Push(args...));
        }

        // 设置一个全局闭包函数
        template<size_t keyLen, typename...Args>
        inline void SetGlobalCFunc(char const(&key)[keyLen], lua_CFunction const &func, Args const &...args) {
            PushFunc(func, args...);
            lua_setglobal(L, key);
        }

    protected:
        // 在 L 里弄个 userdata 出来，将 lambda 挪进去，并设置回收函数
        template<typename F>
        inline void NewLambda(F &&func) {
            auto &&f = (F *) lua_newuserdata(L, sizeof(F));     // ..., ud
            // 将 func 挪进 userdata
            new(f) F(std::forward<F>(func));
            lua_newtable(L);                                    // ..., ud, mt
            lua_pushstring(L, "__gc");                          // ..., ud, mt, "__gc"
            lua_pushvalue(L, -3);                               // ..., ud, mt, "__gc", ud
            lua_pushcclosure(L, [](auto L) {                    // ..., ud, mt, "__gc", cc
                // 从 upvalue 区间取出 userdata 转为入参类型指针 call 之
                auto f = (F *) lua_touserdata(L, lua_upvalueindex(1));
                f->~F();
                return 0;
            }, 1);
            lua_rawset(L, -3);                                  // ..., ud, mt
            lua_setmetatable(L, -2);                            // ..., ud
        }

    public:
        // 设置一个全局 lambda std::function 函数
        template<size_t keyLen, typename T>
        inline void SetGlobalFunc(char const(&key)[keyLen], T &&func) {
            NewLambda(std::forward<T>(func));                   // ..., ud
            lua_pushcclosure(L, [](auto L) {                    // ..., cc
                // 从 upvalue 区间取出 userdata 转为入参类型指针 call 之
                auto &&f = (T *) lua_touserdata(L, lua_upvalueindex(1));
                // 如果 To 出错, 并且 lua 5.3 没有用支持 c++ exception 的方式编译，则 t 无法析构
                FuncA_t<T> tuple;
                // 填充. 如果类型不符将报错退出
                xx::Lua::To(L, 1, tuple);
                int rtv = 0;
                // 参数展开 call 函数
                std::apply([&](auto const &... args) {
                    if constexpr(std::is_void_v<FuncR_t<T>>) {
                        (*f)(args...);
                    } else {
                        rtv = xx::Lua::Push(L, (*f)(args...));
                    }
                }, tuple);
                return rtv;
            }, 1);
            lua_setglobal(L, key);                              // ...
        }


        // 安全调用函数( 函数最先压栈，然后是 up values )
        // [-(nargs + 1), +(nresults|1), -]
        template<typename...Args>
        Result PCall(Args const &...args) {
            Result rtv;
            int n = Push(args...);
            if ((rtv.n = lua_pcall(L, n, LUA_MULTRET, 0))) {
                rtv.m = lua_tostring(L, -1);
                lua_pop(L, 1);
            }
            return rtv;
        }

        // 安全调用指定名称的全局函数( 函数最先压栈，然后是 up values )
        // [-(nargs + 1), +(nresults|1), -]
        template<typename...Args>
        Result PCallGlobalFunc(char const *const &funcName, Args const &...args) {
            GetGlobal(funcName);
            return PCall(args...);
        }

        template<typename...Args>
        Result PCallGlobalFunc(std::string const &funcName, Args const &...args) {
            GetGlobal(funcName);
            return PCall(args...);
        }

        // 不安全调用函数( 函数最先压栈，然后是 up values )
        // [-(nargs + 1), +nresults, e]
        template<typename...Args>
        void Call(Args const &...args) {
            lua_call(L, Push(args...), LUA_MULTRET);
        }

        // 不安全调用指定名称的全局函数( 函数最先压栈，然后是 up values )
        // [-(nargs + 1), +(nresults|1), e]
        template<typename...Args>
        void CallGlobalFunc(char const *const &funcName, Args const &...args) {
            GetGlobal(funcName);
            Call(args...);
        }

        template<typename...Args>
        void CallGlobalFunc(std::string const &funcName, Args const &...args) {
            GetGlobal(funcName);
            Call(args...);
        }

    };
}
