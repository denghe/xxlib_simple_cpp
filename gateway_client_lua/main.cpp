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

__int64__ = 0LL
__uint64__ = 0ULL

function i64()
    return 0LL
end
function u64()
    return 0ULL
end

x = 12345678901234567ULL
x = x * 1.5 * 100LL
print(x)

)-");
    if(r) {
        std::cout << "luaL_dostring r = " << r << ", str = " << lua_tostring(L, -1) << std::endl;
    }

    std::cout << lua_gettop(L) << std::endl;

    // [-0, +1, e]
    lua_getglobal(L, "x");

    // [-0, +0, -]
    auto&& p = lua_topointer(L, -1);

    // 1234567890123456700
    std::cout << *(uint64_t*)p << std::endl;

    // [-0, +0, -]
    std::cout << lua_typename(L, lua_type(L, -1)) << std::endl;

    // [-n, +0, -]
    lua_pop(L, 1);

    std::cout << lua_gettop(L) << std::endl;


    luaL_dostring(L, "return 0LL");
    p = lua_topointer(L, -1);
    *(int64_t*)p = 1;
    lua_setglobal(L, "a");

    r = luaL_dostring(L, R"-(
print(a)
local b = a
a = a + 1
print(a, b)
)-");
    if(r) {
        std::cout << "luaL_dostring r = " << r << ", str = " << lua_tostring(L, -1) << std::endl;
    }

    lua_close(L);

    return 0;
}
