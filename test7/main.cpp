#include <sol/sol.hpp>
#include <cassert>

int main() {
    sol::state lua;
    int x = 0;
    lua.set_function("beep", [&x]{ return ++x; });
    lua.script("beep()");
    sol::function a_function = lua["beep"];
    int v = a_function();
    std::function<int()> a_std_function = a_function;
    int v2 = a_std_function();
    assert(x == 1);
}
