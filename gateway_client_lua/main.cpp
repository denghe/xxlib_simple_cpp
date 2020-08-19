#include "xx_lua.h"
#include "xx_string.h"
#include "xx_chrono.h"
#include <thread>

int main(int argc, char const *argv[]) {
    xx::Lua::Context LC;

    // load
    LC.DoFile("main.lua");
    LC.CheckTop(0);

    // init
    if (auto r = LC.PCallGlobalFunc("Main")) {
        std::cout << r.m << std::endl;
    }

    LC.SetGlobalFunc("Test", [](auto L)->int {
        
        return 0;
    });

    // frame update
    auto lastSecs = xx::NowSteadyEpochSeconds();
    for (int i = 0; i < 100; ++i) {
        std::this_thread::sleep_for(std::chrono::milliseconds(500));
        auto elapsedSecs = xx::NowSteadyEpochSeconds(lastSecs);

        if (auto r = LC.PCallGlobalFunc("UpdateScripts", elapsedSecs)) {
            std::cout << r.m << std::endl;
        }
    }

    return 0;
}















//
//    luaL_dofile(L, "test1.lua");
//    assert(lua_gettop(L) == 1);
//    assert(lua_type(L, -1) == LUA_TTABLE);
//
//    // 找个地方放置 test1.lua return t
//
//    //lua_getglobal(L, "t");          // t
//
//    xx::Data d;
//    xx::DataWriter dw(d);
//    dw.Write(L);
//    xx::CoutN(d);
//
//    lua_pop(L, 1);                  //
//    assert(lua_gettop(L) == 0);
//
//    lua_getglobal(L, "Dump");       // func
//    assert(lua_type(L, -1) == LUA_TFUNCTION);
//
//    xx::DataReader dr(d);
//    int r = dr.Read(L);             // func, t
//    assert(r == 0);
//    assert(lua_gettop(L) == 2);
//    assert(lua_type(L, -1) == LUA_TTABLE);
//
//    if ((r = lua_pcall(L, 1, LUA_MULTRET, 0))) {
//        auto s = lua_tostring(L, -1);
//        std::cout << r << " " << (s ? s : "") << std::endl;
//        lua_pop(L, 1);
//        return r;
//    }


//    xx::Data d;
//    auto ms = xx::NowSteadyEpochMS();
//    for (int i = 0; i < 1000000; ++i) {
//        lua_getglobal(L, "t");          // t
//
//        xx::DataWriter dw(d);
//        dw.Write(L);
//
//        lua_pop(L, 1);                  //
//
//        xx::DataReader dr(d);
//        int r = dr.Read(L);             // t
//
//        lua_pop(L, 1);                  //
//        d.Clear();
//    }
//    xx::CoutN(xx::NowSteadyEpochMS() - ms);


//    std::unordered_map<std::string, int> intint;
//
//    lua_pushlightuserdata(L, &intint);
//    lua_pushcclosure(L, [](lua_State *L) {
//        auto &&intint = *(std::unordered_map<std::string, int> *) lua_topointer(L, lua_upvalueindex(1));
//        auto idx = lua_tostring(L, 1);
//        auto val = lua_tointeger(L, 2);
//        intint[idx] = val;
//        return 0;
//    }, 1);
//    lua_setglobal(L, "SV");
//
//    {
//        auto r = luaL_dostring(L, R"-(
//f1 = function()
//    for i = 1, 100000 do
//        SV(tostring(i), i)
//    end
//end
//
//f2 = function()
//    local t = {}
//    for i = 1, 100000 do
//        t[tostring(i)] = i
//    end
//end
//)-");
//        if (r) {
//            std::cout << "luaL_dostring r = " << r << ", str = " << lua_tostring(L, -1) << std::endl;
//        }
//    }
//
//    // todo: 测试一下映射简单存取值函数到 lua 的性能
//
//    for (int i = 0; i < 10; ++i) {
//        intint.clear();
//        auto bt = xx::NowSteadyEpochMS();
//        lua_getglobal(L, "f1");
//        if (int r = lua_pcall(L, 0, LUA_MULTRET, 0)) {
//            auto s = lua_tostring(L, -1);
//            printf("%d %s", r, (s ? s : ""));
//            lua_pop(L, 1);
//            return r;
//        }
//        xx::CoutN(xx::NowSteadyEpochMS() - bt);
//    }
//    xx::CoutN(intint.size());
//
//    for (int i = 0; i < 10; ++i) {
//        intint.clear();
//        auto bt = xx::NowSteadyEpochMS();
//        lua_getglobal(L, "f2");
//        if (int r = lua_pcall(L, 0, LUA_MULTRET, 0)) {
//            auto s = lua_tostring(L, -1);
//            printf("%d %s", r, (s ? s : ""));
//            lua_pop(L, 1);
//            return r;
//        }
//        xx::CoutN(xx::NowSteadyEpochMS() - bt);
//    }








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