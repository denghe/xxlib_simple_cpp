#pragma once
#include <memory>
#include <cassert>
#include <luajit-2.1/lua.hpp>

namespace xx {
// 被 std::function 捕获携带, 当捕获列表析构发生时, 自动从 L 中反注册函数
    struct Lua_Func {
        // 全局自增函数 id
        inline static int autoIncFuncId = 1;
        inline static char const* key = "xx::Lua_Func";

        // 将函数放入 funcs 表. 保存 key.
        //int funcId = 0;
        std::shared_ptr<int> funcId;
        lua_State* L = nullptr;

        inline explicit operator bool() const {
            return (bool) funcId;
        }

        Lua_Func() = default;

        Lua_Func(lua_State *const &L, int const &idx) {
            if (!idx) return;
            funcId = std::make_shared<int>(autoIncFuncId);
            lua_pushlightuserdata(L, (void *)key);                      // ..., key
            lua_rawget(L, LUA_REGISTRYINDEX);                           // ..., funcs
            lua_pushvalue(L, idx);                                      // ..., funcs, func
            lua_rawseti(L, -2, *funcId);                        // ..., funcs
            lua_pop(L, 1);                                           // ...
            ++autoIncFuncId;
        }

        Lua_Func(Lua_Func const &o) = default;

        Lua_Func(Lua_Func &&o) noexcept
            : funcId(std::move(o.funcId))
            , L(o.L) {
            o.L = nullptr;
        }

        inline Lua_Func &operator=(Lua_Func const &o) = default;

        inline Lua_Func &operator=(Lua_Func &&o)  noexcept {
            std::swap(funcId, o.funcId);
            std::swap(L, o.L);
            return *this;
        }

        // 随 lambda 析构时根据 funcId 删掉函数
        ~Lua_Func() {
            if (!L || funcId.use_count() != 1) return;
            auto &&top = lua_gettop(L);
            lua_pushlightuserdata(L, (void *)key);                      // ..., key
            lua_rawget(L, LUA_REGISTRYINDEX);                           // ..., funcs
            lua_pushnil(L);                                             // ..., funcs, nil
            lua_rawseti(L, -2, *funcId);                        // ..., funcs
            lua_pop(L, 1);                                           // ...
            assert(top == lua_gettop(L));
        }
    };
}
