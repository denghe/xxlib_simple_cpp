#include <sol/sol.hpp>
#include <iostream>

int main() {
    sol::state lua;
    int x = 0;
    lua.set_function("beep", [&x]{ return ++x; });
    lua.script("beep()");
    sol::function sf = lua["beep"];
    std::function<int()> f = sf;
    std::cout << f() << std::endl;
}









#include "xx_typename_islambda.h"
//#include <iostream>
//#include <functional>
//
//int main() {
//    auto f = [&] { return ""; };
//    std::function<int(double)> ff;
//    std::cout << xx::TypeName_v<decltype(f)> << std::endl;
//    std::cout << xx::IsLambda_v<decltype(f)> << std::endl;
//    std::cout << xx::TypeName_v<decltype(ff)> << std::endl;
//    std::cout << xx::IsLambda_v<decltype(ff)> << std::endl;
//    return 0;