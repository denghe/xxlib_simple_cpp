#include "xx_signal.h"
#include <luajit-2.1/lua.hpp>
#include <cassert>
#include <iostream>

int main() {
	// 禁掉 SIGPIPE 信号避免因为连接关闭出错
	xx::IgnoreSignal();

    auto&& L = luaL_newstate();
    assert(L);
    luaL_openlibs(L);
    auto&& r = luaL_dostring(L, R"-(

x = 12345678901234567ULL
x = x * 100

)-");
    if(r) {
        std::cout << "luaL_dostring r = " << r << ", str = " << lua_tostring(L, -1) << std::endl;
    }

    std::cout << lua_gettop(L) << std::endl;

    // [-0, +1, e]
    lua_getglobal(L, "x");
    // [-0, +0, -]
    auto&& p = lua_topointer(L, -1);
    std::cout << *(uint64_t*)p << std::endl;

    // [-0, +0, -]
    std::cout << lua_typename(L, lua_type(L, -1)) << std::endl;

    // [-n, +0, -]
    lua_pop(L, 1);

    std::cout << lua_gettop(L) << std::endl;

    lua_close(L);

    return 0;
}
