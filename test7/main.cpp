#if 1
#include <sol/sol.hpp>
#include <iostream>
#include <chrono>

int main() {
    sol::state lua;
    int x = 0;
    lua.set_function("beep", [&x]{ return ++x; });

    auto t = std::chrono::steady_clock::now();
    lua.script(R"(
for i = 1, 10000000 do
    beep()
end
)");
    std::cout << std::chrono::duration_cast<std::chrono::milliseconds>( std::chrono::steady_clock::now() - t ).count() << std::endl;
    std::cout << x << std::endl;
    t = std::chrono::steady_clock::now();

    sol::function sf = lua["beep"];
    std::function<int()> f = sf;
    for (int i = 0; i < 10000000; ++i) {
        f();
    }
    std::cout << std::chrono::duration_cast<std::chrono::milliseconds>( std::chrono::steady_clock::now() - t ).count() << std::endl;
    std::cout << x << std::endl;
}

#else

#include <xx_lua.h>
#include <iostream>
#include <chrono>

namespace XL = xx::Lua;

int main() {
    xx::Lua::State L;
    if (auto r = XL::Try(L, [&] {
        int x = 0;
        XL::SetGlobal(L, "beep", [&x] { return ++x; });

        auto t = std::chrono::steady_clock::now();
        luaL_dostring(L, R"(
for i = 1, 10000000 do
    beep()
end
)");
        std::cout << std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now() - t).count() << std::endl;
        std::cout << x << std::endl;
        t = std::chrono::steady_clock::now();

        std::function<int()> f;
        XL::GetGlobal(L, "beep", f);
        for (int i = 0; i < 10000000; ++i) {
            f();
        }
        std::cout << std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now() - t).count() << std::endl;
        std::cout << x << std::endl;
    })) {
        std::cout << "error! " << r.m << std::endl;
    }
}

#endif