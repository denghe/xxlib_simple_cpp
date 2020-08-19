#pragma once

#include "xx_data_rw.h"
#include <lua.hpp>

namespace xx {
    // Lua 简单数据结构 序列化适配( 不支持 table 循环引用, 不支持 lua5.1 int64, 整数需在 int32 范围内 )
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

    // Lua push 系列基础适配模板. Push 返回实际入栈的参数个数( 通常是 1. 但如果传入一个队列并展开压栈则不一定 )
    template<typename T, typename ENABLED = void>
    struct PushFuncs {
        static inline int Push(lua_State *const &L, T const &in) {
            throw std::runtime_error("Lua push: not support's type");
            return 0;
        }
    };

    // 适配 bool
    template<>
    struct PushFuncs<bool, void> {
        static inline int Push(lua_State *const &L, bool const &in) {
            lua_checkstack(L, 1);
            lua_pushboolean(L, in ? 1 : 0);
            return 1;
        }
    };

    // 适配 整数
    template<typename T>
    struct PushFuncs<T, std::enable_if_t<std::is_integral_v<T>>> {
        static inline int Push(lua_State *const &L, T const &in) {
            lua_checkstack(L, 1);
            lua_pushinteger(L, in);
            return 1;
        }
    };

    // 适配 枚举( 转为整数 )
    template<typename T>
    struct PushFuncs<T, std::enable_if_t<std::is_enum_v<T>>> {
        typedef std::underlying_type_t<T> UT;

        static inline int Push(lua_State *const &L, T const &in) {
            return PushFuncs<UT, void>::Push(L, (UT) in);
        }
    };

    // 适配 浮点
    template<typename T>
    struct PushFuncs<T, std::enable_if_t<std::is_floating_point_v<T>>> {
        static inline int Push(lua_State *const &L, T const &in) {
            lua_checkstack(L, 1);
            lua_pushnumber(L, in);
            return 1;
        }
    };

    // 适配 literal char[len] string
    template<size_t len>
    struct PushFuncs<char[len], void> {
        static inline int Push(lua_State *const &L, char const(&in)[len]) {
            lua_checkstack(L, 1);
            lua_pushlstring(L, in, len - 1);
            return 1;
        }
    };

    // 适配 std::string 或 std::string_view
    template<typename T>
    struct PushFuncs<T, std::enable_if_t<std::is_same_v<T, std::string> || std::is_same_v<T, std::string_view>>> {
        static inline int Push(lua_State *const &L, T const &in) {
            lua_checkstack(L, 1);
            lua_pushlstring(L, in.data(), in.size());
            return 1;
        }
    };

    // 适配 char*, char const*
    template<typename T>
    struct PushFuncs<T, std::enable_if_t<std::is_same_v<T, char *> || std::is_same_v<T, char const *>>> {
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
            return xx::Lua::PushFuncs<Arg>::Push(L, arg);
        }

        template<typename Arg, typename...Args>
        int Push(lua_State *const &L, Arg const &arg, Args const &...args) {
            int n = xx::Lua::PushFuncs<Arg>::Push(L, arg);
            return n + Push(L, args...);
        }
    }

    template<typename...Args>
    int Push(lua_State *const &L, Args const &...args) {
        return xx::Lua::Detail::Push(L, args...);
    }

    // pcall 返回值封装, 易于使用
    struct Result {
        int r = 0;
        char const *m = nullptr;

        inline operator bool() const {
            return r != 0;
        }
    };

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

        // [-0, +0, -]
        inline int GetTop() {
            return lua_gettop(L);
        }

        inline void CheckTop(int const &n, char const *const &msg = "wrong top value") {
            if (GetTop() != n) throw std::logic_error(msg);
        }

        // [-0, +1, e]
        inline void GetGlobal(char const *const &key) {
            lua_getglobal(L, key);
        }

        // [-1, +0, e]
        inline void SetGlobal(char const *const &key) {
            lua_setglobal(L, key);
        }

        // [-0, +?, m]
        inline void DoFile(char const *const &fileName) {
            luaL_dofile(L, fileName);
        }

        template<typename...Args>
        int Push(Args const &...args) {
            return xx::Lua::Push(L, args...);
        }

        int Push() { return 0; }

        // [-n, +1, m]
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

        // 设置一个全局 lambda std::function 函数
        // 思路：利用 up value 传递 std::function 的指针存于 user data 里，设置 metatable ，通过 __gc 来删
        template<size_t keyLen, typename FR, typename...FArgs, typename...Args>
        inline void SetGlobalFunc(char const(&key)[keyLen], std::function<FR(FArgs const &...)> &&func) {
            // todo: 先申请 userdata 放置 func 的指针
            lua_newtable(L);                                    // ..., mt
            lua_pushstring(L, "__gc");                          // ..., mt, "__gc"
            // todo: 将 func 指针作为 up value 带入 __gc 闭包
            lua_pushcclosure(L, [](auto) {
                // todo: 取出闭包数据，硬转为 函数指针, delete 之
            }, 1);
            // lua_setmetatable(L, ??)
//            SetGlobalCFunc(key, [](auto)->int{
//            }, args...);
        }


        // [-(nargs + 1), +(nresults|1), -]
        template<typename...Args>
        Result PCall(Args const &...args) {
            Result rtv;
            int n = Push(args...);
            if ((rtv.r = lua_pcall(L, n, LUA_MULTRET, 0))) {
                rtv.m = lua_tostring(L, -1);
                lua_pop(L, 1);
            }
            return rtv;
        }

        // [-(nargs + 1), +(nresults|1), -]
        template<typename...Args>
        Result PCallGlobalFunc(char const *const &funcName, Args const &...args) {
            GetGlobal(funcName);
            return PCall(args...);
        }



        // todo: Pop?
    };
}

