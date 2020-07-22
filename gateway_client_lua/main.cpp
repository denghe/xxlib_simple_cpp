#include "xx_signal.h"
//#include <luajit-2.1/lua.hpp>
#include <lua5.3/lua.hpp>
#include <cassert>
#include <iostream>
#include <map>
#include "xx_string.h"
#include "xx_chrono.h"

int main() {
    auto &&L = luaL_newstate();
    assert(L);
    luaL_openlibs(L);

    std::unordered_map<std::string, int> intint;

    lua_pushlightuserdata(L, &intint);
    lua_pushcclosure(L, [](lua_State *L) {
        auto &&intint = *(std::unordered_map<std::string, int> *) lua_topointer(L, lua_upvalueindex(1));
        auto idx = lua_tostring(L, 1);
        auto val = lua_tointeger(L, 2);
        intint[idx] = val;
        return 0;
    }, 1);
    lua_setglobal(L, "SV");

    {
        auto r = luaL_dostring(L, R"-(
f1 = function()
    for i = 1, 100000 do
        SV(tostring(i), i)
    end
end

f2 = function()
    local t = {}
    for i = 1, 100000 do
        t[tostring(i)] = i
    end
end
)-");
        if (r) {
            std::cout << "luaL_dostring r = " << r << ", str = " << lua_tostring(L, -1) << std::endl;
        }
    }

    // todo: 测试一下映射简单存取值函数到 lua 的性能

    for (int i = 0; i < 10; ++i) {
        intint.clear();
        auto bt = xx::NowSteadyEpochMS();
        lua_getglobal(L, "f1");
        if (int r = lua_pcall(L, 0, LUA_MULTRET, 0)) {
            auto s = lua_tostring(L, -1);
            printf("%d %s", r, (s ? s : ""));
            lua_pop(L, 1);
            return r;
        }
        xx::CoutN(xx::NowSteadyEpochMS() - bt);
    }
    xx::CoutN(intint.size());

    for (int i = 0; i < 10; ++i) {
        intint.clear();
        auto bt = xx::NowSteadyEpochMS();
        lua_getglobal(L, "f2");
        if (int r = lua_pcall(L, 0, LUA_MULTRET, 0)) {
            auto s = lua_tostring(L, -1);
            printf("%d %s", r, (s ? s : ""));
            lua_pop(L, 1);
            return r;
        }
        xx::CoutN(xx::NowSteadyEpochMS() - bt);
    }

    lua_close(L);
    return 0;
}


//--[[
//local count = 0
//for _, _ in pairs(t) do
//   count = count + 1
//end
//print(count)
//]]

//
//    auto&& r = luaL_dostring(L, R"-(
//
//__int64__ = 0LL
//__uint64__ = 0ULL
//
//function i64()
//    return 0LL
//end
//function u64()
//    return 0ULL
//end
//
//x = 12345678901234567ULL
//x = x * 1.5 * 100LL
//print(x)
//
//)-");
//    if(r) {
//        std::cout << "luaL_dostring r = " << r << ", str = " << lua_tostring(L, -1) << std::endl;
//    }
//
//    std::cout << lua_gettop(L) << std::endl;
//
//    // [-0, +1, e]
//    lua_getglobal(L, "x");
//
//    // [-0, +0, -]
//    auto&& p = lua_topointer(L, -1);
//
//    // 1234567890123456700
//    std::cout << *(uint64_t*)p << std::endl;
//
//    // [-0, +0, -]
//    std::cout << lua_typename(L, lua_type(L, -1)) << std::endl;
//
//    // [-n, +0, -]
//    lua_pop(L, 1);
//
//    std::cout << lua_gettop(L) << std::endl;
//
//
//    luaL_dostring(L, "return 0LL");
//    p = lua_topointer(L, -1);
//    *(int64_t*)p = 1;
//    lua_setglobal(L, "a");
//
//    r = luaL_dostring(L, R"-(
//print(a)
//local b = a
//a = a + 1
//print(a, b)
//)-");
//    if(r) {
//        std::cout << "luaL_dostring r = " << r << ", str = " << lua_tostring(L, -1) << std::endl;
//    }