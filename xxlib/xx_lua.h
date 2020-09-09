#pragma once

// 与 lua 交互的代码应 Try 执行, 方能正确响应 luaL_error 或 C++ 异常
// luajit 支持 C++ 异常, 支持 中文变量名, 官方 lua 5.3/4 很多预编译库默认不支持, 必须强制以 C++ 方式自己编译
// 注意：To 系列 批量操作时，不支持负数 idx ( 递归模板里 + 1 就不对了 )

#include "xx_data_rw.h"
#include "xx_string.h"
#include "xx_typename_islambda.h"

#ifndef MAKE_LIB
extern "C" {
#endif

#include "lua.h"
#include "lualib.h"
#include "lauxlib.h"

#ifndef MAKE_LIB
}
#endif


namespace xx::Lua {

    // for easy use
    template<typename...Args>
    inline int Error(lua_State *const &L, Args const &... args) {
        return luaL_error(L, ::xx::ToString(args...).c_str());
    }

    // for easy push
    struct Nil {
    };

    // 如果是 luajit 就啥都不用做了
    int CheckStack(lua_State *const &L, int const &n) {
#ifndef LUAJIT_VERSION
        return lua_checkstack(L, n);
#else
        return 1;
#endif
    }


    /******************************************************************************************************************/
    template<typename T, typename ENABLED = void>
    struct MetatableFillFuncs {
        static inline void Fill(lua_State *const &L) {}
    };

    template<typename T, typename ENABLED = void>
    struct MetatablePushFuncs;

    template<typename T>
    struct MetatablePushFuncs<T, void> {
        inline static int refId = -1;

        static inline int Push(lua_State *const &L) {
            if (refId == -1) {
                CheckStack(L, 1);
                lua_createtable(L, 0, 20);                                      // ..., mt

                if constexpr(!std::is_pod_v<T>) {
                    lua_pushstring(L, "__gc");                                  // ..., mt, "__gc"
                    lua_pushcclosure(L, [](auto L) {                            // ..., mt, "__gc", cc
                        auto f = (T *) lua_touserdata(L, -1);
                        f->~T();
                        return 0;
                    }, 0);
                    lua_rawset(L, -3);                                          // ..., mt
                }

                if constexpr(!xx::IsLambda_v<T>) {
                    lua_pushstring(L, "__index");                               // ..., mt, "__index"
                    lua_pushvalue(L, -2);                                       // ..., mt, "__index", mt
                    lua_rawset(L, -3);                                          // ..., mt
                }

                MetatableFillFuncs<T, void>::Fill(L);                           // ..., mt

                lua_pushvalue(L, -1);                                           // ..., mt, mt
                refId = luaL_ref(L, LUA_REGISTRYINDEX);                         // ..., mt
            } else {
                lua_rawgeti(L, LUA_REGISTRYINDEX, refId);                       // ..., mt
            }
            return 1;
        }
    };

    template<typename T>
    int PushMetatable(lua_State *const &L, bool const &_gc = true, bool const &_index = true) {
        return MetatablePushFuncs<T>::Push(L, _gc, _index);
    }


    /******************************************************************************************************************/
    // Lua push, to 系列基础适配模板. Push 返回实际入栈的参数个数( 通常是 1. 但如果传入一个队列并展开压栈则不一定 ). To 无返回值.
    // 可能抛 lua 异常( 故这些函数应该间接被 pcall 使用 )
    template<typename T, typename ENABLED = void>
    struct PushToFuncs {
        static inline int Push(lua_State *const &L, T const &in) {
            return Error(L, "Lua Push error! not support's type ", xx::TypeName_v<T>);
        }

        static inline void To(lua_State *const &L, int const &idx, T &out) {
            Error(L, "Lua Push error! not support's type ", xx::TypeName_v<T>);
        }
    };

    // 适配 nil 写入
    template<>
    struct PushToFuncs<Nil, void> {
        static inline int Push(lua_State *const &L, bool const &in) {
            CheckStack(L, 1);
            lua_pushnil(L);
            return 1;
        }
    };

    // 适配 bool
    template<>
    struct PushToFuncs<bool, void> {
        static inline int Push(lua_State *const &L, bool const &in) {
            CheckStack(L, 1);
            lua_pushboolean(L, in ? 1 : 0);
            return 1;
        }

        static inline void To(lua_State *const &L, int const &idx, bool &out) {
            if (!lua_isboolean(L, idx)) Error(L, "error! args[", idx, "] is not bool");
            out = (bool) lua_toboolean(L, idx);
        }
    };

    // 适配 整数
    template<typename T>
    struct PushToFuncs<T, std::enable_if_t<std::is_integral_v<T>>> {
        static inline int Push(lua_State *const &L, T const &in) {
            CheckStack(L, 1);
            lua_pushinteger(L, in);
            return 1;
        }

        static inline void To(lua_State *const &L, int const &idx, T &out) {
            int isnum = 0;
            out = (T) lua_tointegerx(L, idx, &isnum);
            if (!isnum) Error(L, "error! args[", idx, "] is not number");
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
            CheckStack(L, 1);
            lua_pushnumber(L, in);
            return 1;
        }

        static inline void To(lua_State *const &L, int const &idx, T &out) {
            int isnum = 0;
            out = (T) lua_tonumberx(L, idx, &isnum);
            if (!isnum) Error(L, "error! args[", idx, "] is not number");
        }
    };

    // 适配 literal char[len] string
    template<size_t len>
    struct PushToFuncs<char[len], void> {
        static inline int Push(lua_State *const &L, char const(&in)[len]) {
            CheckStack(L, 1);
            lua_pushlstring(L, in, len - 1);
            return 1;
        }
    };

    // 适配 std::string
    template<>
    struct PushToFuncs<std::string, void> {
        static inline int Push(lua_State *const &L, std::string const &in) {
            CheckStack(L, 1);
            lua_pushlstring(L, in.data(), in.size());
            return 1;
        }

        static inline void To(lua_State *const &L, int const &idx, std::string &out) {
            if (!lua_isstring(L, idx)) Error(L, "error! args[", idx, "] is not string");
            size_t len = 0;
            auto ptr = lua_tolstring(L, idx, &len);
            out.assign(ptr, len);
        }
    };

    // 适配 std::string_view
    template<>
    struct PushToFuncs<std::string_view, void> {
        static inline int Push(lua_State *const &L, std::string_view const &in) {
            CheckStack(L, 1);
            lua_pushlstring(L, in.data(), in.size());
            return 1;
        }
    };

    // 适配 char*, char const*
    template<typename T>
    struct PushToFuncs<T, std::enable_if_t<std::is_same_v<T, char *> || std::is_same_v<T, char const *>>> {
        static inline int Push(lua_State *const &L, T const &in) {
            CheckStack(L, 1);
            lua_pushstring(L, in);
            return 1;
        }

        static inline void To(lua_State *const &L, int const &idx, T &out) {
            if (!lua_isstring(L, idx)) Error(L, "error! args[", idx, "] is not string");
            out = (T) lua_tostring(L, idx);
        }
    };

    // 适配 std::optional
    template<typename T>
    struct PushToFuncs<std::optional<T>, void> {
        static inline int Push(lua_State *const &L, std::optional<T> const &in) {
            CheckStack(L, 1);
            if (in.has_value()) {
                PushToFuncs<T>::Push(in.value());
            } else {
                lua_pushnil(L);
            }
            return 1;
        }

        static inline void To(lua_State *const &L, int const &idx, std::optional<T> &out) {
            if (lua_isnil(L, idx)) {
                out.reset();
            } else {
                PushToFuncs<T>::To(L, idx, out.value());
            }
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

        int Push(lua_State *const &L) {
            return 0;
        }
    }

    template<typename...Args>
    int Push(lua_State *const &L, Args const &...args) {
        return ::xx::Lua::Detail::Push(L, args...);
    }

    int Push(lua_State *const &L) {
        return 0;
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
                rtv = xx::Lua::Push(L, args...);
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
    // 常用交互函数封装

    // 向 idx 的 table 写入 k, v
    template<typename K, typename V>
    inline void SetField(lua_State *const &L, int const &idx, K const &k, V const &v) {
        Push(L, k, v);                          // ..., table at idx, ..., k, v
        lua_rawset(L, idx);                     // ..., table at idx, ...
    }

    // 向 栈顶 的 table 写入 k, v
    template<typename K, typename V>
    inline void SetField(lua_State *const &L, K const &k, V const &v) {
        Push(L, k, v);                          // ..., table at idx, ..., k, v
        lua_rawset(L, -3);                      // ..., table at idx, ...
    }


    // 根据 k 从 idx 的 table 读出 v
    template<typename K, typename V>
    inline void GetField(lua_State *const &L, int const &idx, K const &k, V &v) {
        auto top = lua_gettop(L);
        Push(L, k);                             // ..., table at idx, ..., k
        lua_rawget(L, idx);                     // ..., table at idx, ..., v
        To(L, top + 1, v);
        lua_settop(L, top);                     // ..., table at idx, ...
    }

    // 写 k, v 到全局
    template<typename K, typename V>
    inline void SetGlobal(lua_State *const &L, K const &k, V const &v) {
        Push(L, v);
        if constexpr(std::is_same_v<K, std::string> || std::is_same_v<K, std::string_view>) {
            lua_setglobal(L, k.c_str());
        } else {
            lua_setglobal(L, k);
        }
    }

    // 从全局以 k 读 v
    template<typename K, typename V>
    inline void GetGlobal(lua_State *const &L, K const &k, V &v) {
        auto top = lua_gettop(L);
        if constexpr(std::is_same_v<K, std::string> || std::is_same_v<K, std::string_view>) {
            lua_getglobal(L, k.c_str());
        } else {
            lua_getglobal(L, k);
        }
        To(L, top + 1, v);
        lua_settop(L, top);
    }

    // 写 k, v 到注册表
    template<typename K, typename V>
    inline void SetRegistry(lua_State *const &L, K const &k, V const &v) {
        SetField(L, LUA_REGISTRYINDEX, k, v);
    }

    // 从注册表以 k 读 v
    template<typename K, typename V>
    inline void GetRegistry(lua_State *const &L, K const &k, V const &v) {
        GetField(L, LUA_REGISTRYINDEX, k, v);
    }

    // 压入一个 T( 内容复制到 userdata, 且注册 mt )
    template<typename T>
    void PushUserdata(lua_State *const &L, T &&v) {
        using U = std::decay_t<T>;
        CheckStack(L, 2);
        auto f = (U *) lua_newuserdata(L, sizeof(U));                   // ..., ud
        new(f) U(std::forward<T>(v));
        MetatablePushFuncs<U>::Push(L);                                 // ..., ud, mt
        lua_setmetatable(L, -2);                                        // ..., ud
    }

    // 适配 lambda
    template<typename T>
    struct PushToFuncs<T, std::enable_if_t<xx::IsLambda_v<T>>> {
        static inline int Push(lua_State *const &L, T const &in) {
            PushUserdata(L, in);                                        // ..., ud
            lua_pushcclosure(L, [](auto L) {                            // ..., cc
                auto f = (T *) lua_touserdata(L, lua_upvalueindex(1));
                LambdaArgs_t<T> tuple;
                To(L, 1, tuple);
                int rtv = 0;
                std::apply([&](auto const &... args) {
                    if constexpr(std::is_void_v<LambdaRtv_t<T>>) {
                        (*f)(args...);
                    } else {
                        rtv = xx::Lua::Push(L, (*f)(args...));
                    }
                }, tuple);
                return rtv;
            }, 1);
            return 1;
        }
    };


    // 被 std::function 捕获携带, 当捕获列表析构发生时, 自动从 L 中反注册函数
    // 需自己确保这些 function 活的比 L 久
    struct FuncWrapper {
        // 将函数以 luaL_ref 方式放入注册表.
        std::shared_ptr<std::pair<lua_State *, int>> p;

        FuncWrapper() = default;

        FuncWrapper(FuncWrapper const &o) = default;

        FuncWrapper &operator=(FuncWrapper const &o) = default;

        FuncWrapper(FuncWrapper &&o) noexcept
                : p(std::move(o.p)) {
        }

        inline FuncWrapper &operator=(FuncWrapper &&o) noexcept {
            std::swap(p, o.p);
            return *this;
        }

        FuncWrapper(lua_State *const &L, int const &idx) {
            if (!lua_isfunction(L, idx)) Error(L, "args[", idx, "] is not a lua function");
            CheckStack(L, 1);
            xx::MakeTo(p);
            p->first = L;
            lua_pushvalue(L, idx);                                      // ..., func, ..., func
            p->second = luaL_ref(L, LUA_REGISTRYINDEX);                 // ..., func, ...
        }

        // 如果 p 引用计数唯一, 则反注册
        ~FuncWrapper() {
            if (p.use_count() != 1) return;
            luaL_unref(p->first, LUA_REGISTRYINDEX, p->second);
        }
    };


    // 适配 std::function
    template<typename T>
    struct PushToFuncs<std::function<T>, void> {
        static inline int Push(lua_State *const &L, std::function<T> const &in) {
            PushUserdata(L, in);                                        // ..., ud
            lua_pushcclosure(L, [](auto L) {                            // ..., cc
                auto f = (std::function<T> *) lua_touserdata(L, lua_upvalueindex(1));
                LambdaArgs_t<T> tuple;
                To(L, 1, tuple);
                int rtv = 0;
                std::apply([&](auto const &... args) {
                    if constexpr(std::is_void_v<LambdaRtv_t<T>>) {
                        (*f)(args...);
                    } else {
                        rtv = xx::Lua::Push(L, (*f)(args...));
                    }
                }, tuple);
                return rtv;
            }, 1);
            return 1;
        }

        static inline void To(lua_State *const &L, int const &idx, std::function<T> &out) {
            out = [fw = FuncWrapper(L, idx)](auto... args) {
                auto &&L = fw.p->first;
                auto top = lua_gettop(L);
                CheckStack(L, 1);
                lua_rawgeti(L, LUA_REGISTRYINDEX, fw.p->second);            // ..., func
                auto num = ::xx::Lua::Push(L, args...);                     // ..., func, args...
                lua_call(L, num, LUA_MULTRET);                              // ..., rtv...?
                if constexpr(!std::is_void_v<LambdaRtv_t<T>>) {
                    LambdaRtv_t<T> rtv;
                    xx::Lua::To(L, top + 1, rtv);
                    lua_settop(L, top);                                     // ...
                    return rtv;
                } else {
                    lua_settop(L, top);                                     // ...( 保险起见 )
                }
            };
        }
    };


    // 适配 T*
    template<typename T>
    struct PushToFuncs<T *, std::enable_if_t<!std::is_same_v<std::decay_t<T>, char> && !std::is_same_v<std::decay_t<T>, char const>>> {
        using U = T *;

        static inline int Push(lua_State *const &L, U const &in) {
            PushUserdata(L, in);                                                        // ..., ud
            return 1;
        }

        static inline void To(lua_State *const &L, int const &idx, U &out) {
            if (!lua_isuserdata(L, idx)) goto LabError;
            CheckStack(L, 2);
            lua_getmetatable(L, idx);                                                   // ... tar(idx) ..., mt
            lua_rawgeti(L, LUA_REGISTRYINDEX, MetatablePushFuncs<U>::refId);            // ... tar(idx) ..., mt, mt
            if (!lua_rawequal(L, -1, -2)) goto LabError;
            lua_pop(L, 2);                                                              // ... tar(idx) ...
            out = *(U *) lua_touserdata(L, idx);
            return;
            LabError:
            Error(L, "error! args[", idx, "] is not ", xx::TypeName_v<U>);
        }
    };

    // 适配 std::shared_ptr<T>
    template<typename T>
    struct PushToFuncs<std::shared_ptr<T>, void> {
        using U = std::shared_ptr<T>;

        static inline int Push(lua_State *const &L, U const &in) {
            PushUserdata(L, in);                                                        // ..., ud
            return 1;
        }

        static inline void To(lua_State *const &L, int const &idx, U &out) {
            if (!lua_isuserdata(L, idx)) goto LabError;
            CheckStack(L, 2);
            lua_getmetatable(L, idx);                                                   // ... tar(idx) ..., mt
            lua_rawseti(L, LUA_REGISTRYINDEX, MetatablePushFuncs<U>::refId);            // ... tar(idx) ..., mt, mt
            if (!lua_rawequal(L, -1, -2)) goto LabError;
            lua_pop(L, 2);                                                              // ... tar(idx) ...
            out = *(U *) lua_touserdata(L, idx);
            return;
            LabError:
            Error(L, "error! args[", idx, "] is not ", xx::TypeName_v<U>);
        }
    };



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

    // 安全执行 lambda。不负责还原 top. 如果中途有 luaL_error 则 返回错误码 和 错误文本。理论上讲所有 lua 调用都该包这个
    template<typename T>
    Result Try(lua_State *const &L, T &&func) {
        Result rtv;
        if (!CheckStack(L, 2)) {
            rtv.n = -1;
            rtv.m = "lua_checkstack(L, 1) failed. not enough memory??";
            return rtv;
        }
        lua_pushlightuserdata(L, &func);                                // ..., &func
        lua_pushcclosure(L, [](auto L) {                                // ..., cfunc
            auto &&f = (T *) lua_touserdata(L, lua_upvalueindex(1));
            (*f)();
            return 0;
        }, 1);
        if ((rtv.n = lua_pcall(L, 0, LUA_MULTRET, 0))) {                // ...
            rtv.m = lua_tostring(L, -1);
        }
        return rtv;
    }

    template<typename...Args>
    void AssertTop(lua_State *const &L, int const &n, Args const &...args) {
        auto top = lua_gettop(L);
        if (top != n) Error(L, "AssertTop( ", n, " ) failed! current top = ", top, args...);
    }

    // 方便在 do string 的时候拼接字串
    template<typename...Args>
    void DoString(lua_State *const &L, Args const &...args) {
        luaL_dostring(L, xx::ToString(args...).c_str());
    }


    // 安全调用函数( 函数最先压栈，然后是 up values )
    // [-(nargs + 1), +(nresults|1), -]
    template<typename...Args>
    Result PCall(lua_State *const &L, Args const &...args) {
        Result rtv;
        int n = Push(L, args...);
        if ((rtv.n = lua_pcall(L, n, LUA_MULTRET, 0))) {
            rtv.m = lua_tostring(L, -1);
            lua_pop(L, 1);
        }
        return rtv;
    }

    // 安全调用指定名称的全局函数( 函数最先压栈，然后是 up values )
    // [-(nargs + 1), +(nresults|1), -]
    template<typename...Args>
    Result PCallGlobalFunc(lua_State *const &L, char const *const &funcName, Args const &...args) {
        lua_getglobal(L, funcName);
        return PCall(L, args...);
    }

    template<typename...Args>
    Result PCallGlobalFunc(lua_State *const &L, std::string const &funcName, Args const &...args) {
        lua_getglobal(L, funcName.c_str());
        return PCall(L, args...);
    }

    // 不安全调用函数( 函数最先压栈，然后是 up values )
    // [-(nargs + 1), +nresults, e]
    template<typename...Args>
    void Call(lua_State *const &L, Args const &...args) {
        lua_call(L, Push(L, args...), LUA_MULTRET);
    }

    // 不安全调用指定名称的全局函数( 函数最先压栈，然后是 up values )
    // [-(nargs + 1), +(nresults|1), e]
    template<typename...Args>
    void CallGlobalFunc(lua_State *const &L, char const *const &funcName, Args const &...args) {
        lua_getglobal(L, funcName);
        Call(L, args...);
    }

    template<typename...Args>
    void CallGlobalFunc(lua_State *const &L, std::string const &funcName, Args const &...args) {
        lua_getglobal(L, funcName.c_str());
        Call(L, args...);
    }

    /******************************************************************************************************************/
    // Lua State 简单封装, 可直接当指针用, 离开范围自动 close
    struct State {
        lua_State *L = nullptr;

        inline operator lua_State *() {
            return L;
        }

        ~State() {
            lua_close(L);
        }

        explicit State(bool const &openLibs = true) {
            L = luaL_newstate();
            if (!L) throw std::logic_error("auto &&L = luaL_newstate(); if (!L)");
            if (openLibs) {
                luaL_openlibs(L);
            }
        }

        explicit State(lua_State *L) : L(L) {}

        State(State const &) = delete;

        State &operator=(State const &) = delete;

        State(State &&o) noexcept: L(o.L) {
            o.L = nullptr;
        }

        State &operator=(State &&o) noexcept {
            std::swap(L, o.L);
            return *this;
        }
    };
}


namespace xx {
    /******************************************************************************************************************/
    // Lua 简单数据结构 序列化适配( 不支持 table 循环引用, 注意 lua5.1 不支持 int64, 整数需在 int32 范围内 )
    // 并非规范接口，Read 是从 L 栈顶提取，Write 是新增到 L 栈顶

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
#if defined(LUA_VERSION_NUM) && LUA_VERSION_NUM >= 503
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
                    Lua::CheckStack(in, 1);
                    lua_pushnil(in);                                            //                      ... t, nil
                    while (lua_next(in, idx) != 0) {                            //                      ... t, k, v
                        // 先检查下 k, v 是否为不可序列化类型. 如果是就 next
                        t = lua_type(in, -1);
                        if (t == LUA_TFUNCTION || t == LUA_TLIGHTUSERDATA || t == LUA_TUSERDATA || t == LUA_TTHREAD) {
                            lua_pop(in, 1);                                     //                      ... t, k
                            continue;
                        }
                        t = lua_type(in, -2);
                        if (t == LUA_TFUNCTION || t == LUA_TLIGHTUSERDATA || t == LUA_TUSERDATA || t == LUA_TTHREAD) {
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
                    Lua::CheckStack(out, 1);
                    lua_pushnil(out);
                    return 0;
                case LuaTypes::True:
                    Lua::CheckStack(out, 1);
                    lua_pushboolean(out, 1);
                    return 0;
                case LuaTypes::False:
                    Lua::CheckStack(out, 1);
                    lua_pushboolean(out, 0);
                    return 0;
                case LuaTypes::Integer: {
                    int64_t i;
                    if (int r = dr.Read(i)) return r;
                    Lua::CheckStack(out, 1);
                    lua_pushinteger(out, (lua_Integer) i);
                    return 0;
                }
                case LuaTypes::Double: {
                    lua_Number d;
                    if (int r = dr.ReadFixed(d)) return r;
                    Lua::CheckStack(out, 1);
                    lua_pushnumber(out, d);
                    return 0;
                }
                case LuaTypes::String: {
                    size_t len;
                    if (int r = dr.Read(len)) return r;
                    Lua::CheckStack(out, 1);
                    lua_pushlstring(out, dr.buf + dr.offset, len);
                    dr.offset += len;
                    return 0;
                }
                case LuaTypes::Table: {
                    Lua::CheckStack(out, 4);
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

    template<>
    struct DataFuncs<::xx::Lua::State, void> {
        static inline void Write(DataWriter &dw, ::xx::Lua::State const &in) {
            ::xx::DataFuncs<lua_State *, void>::Write(dw, in.L);
        }

        static inline int Read(DataReader &dr, ::xx::Lua::State &out) {
            return ::xx::DataFuncs<lua_State *, void>::Read(dr, out.L);
        }
    };
}
