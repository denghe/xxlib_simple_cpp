﻿#include "xx_point.h"

int main() {
    auto &&pathway = xx::PathwayMaker({0, 0})
            .RotateTo(M_PI_4)
            .Forward(10)
            .RotateBy(M_PI_4)
            .Forward(10)
            .Loop();
    xx::CoutN(*pathway);
    return 0;
}





























//#include <iostream>
//#include <vector>
//#include <cstring>
//#include <array>
//
//constexpr size_t numRows = 20, numCols = 50;
//std::array<std::array<char, 51>, numRows> img = {
//        "01234567890123456789012345678901234567890123456789",
//        "01234567890123456789012345678901234567890123456789",
//        "01234567890123456789012345678901234567890123456789",
//        "01234567890123456789012345678901234567890123456789",
//        "01234567890123456789012345678901234567890123456789",
//        "01234567890123456789012345678901234567890123456789",
//        "01234567890123+-----+12345678901234567890123456789",
//        "01234567890123|     |12345678901234567890123456789",
//        "01234567890123|     |12345678901234567890123456789",
//        "01234567890123|     |12345678901234567890123456789",
//        "01234567890123+-----+12345678901234567890123456789",
//        "01234567890123456789012345678901234567890123456789",
//        "01234567890123456789012345678901234567890123456789",
//        "01234567890123456789012345678+-----+67890123456789",
//        "01234567890123456789012345678|     |67890123456789",
//        "01234567890123456789012345678|     |67890123456789",
//        "01234567890123456789012345678|     |67890123456789",
//        "01234567890123456789012345678+-----+67890123456789",
//        "01234567890123456789012345678901234567890123456789",
//        "01234567890123456789012345678901234567890123456789",
//};
//constexpr size_t numRows2 = 5, numCols2 = 7;
//std::array<std::array<char, 8>, numRows2> img2 = {
//        "+-----+",
//        "|     |",
//        "|     |",
//        "|     |",
//        "+-----+",
//};
//
//template<typename IMG>
//void Dump(IMG const &img_, size_t const &numRows_, size_t const &numCols_) {
//    for (int i = 0; i < numRows_; ++i) {
//        for (int j = 0; j < numCols_; ++j) {
//            std::cout << img_[i][j];
//        }
//        std::cout << std::endl;
//    }
//}
//
//template<typename IMG1, typename IMG2>
//void Find1(std::vector<std::pair<size_t, size_t>> &rtv, IMG1 const &img_, size_t const &numRows_, size_t const &numCols_, IMG2 const &img2_, size_t const &numRows2_,
//           size_t const &numCols2_) {
//    rtv.clear();
//    for (int i = 0; i < numRows_ - numRows2_; ++i) {
//        for (int j = 0; j < numCols_ - numCols2_; ++j) {
//            for (int k = 0; k < numRows2_; ++k) {
//                if (memcmp(&img_[i + k][j], &img2_[k][0], numCols2_ * sizeof(img2_[k][0])) != 0) {
//                    goto LabContinue;
//                }
//            }
//            rtv.emplace_back(i, j);
//            LabContinue:;
//        }
//    }
//}
//
//
//int main() {
//    Dump(img, numRows, numCols);
//    Dump(img2, numRows2, numCols2);
//    std::vector<std::pair<size_t, size_t>> rtv;
//    Find1(rtv, img, numRows, numCols, img2, numRows2, numCols2);
//    for (auto &&ij : rtv) {
//        std::cout << ij.first << "," << ij.second << std::endl;
//    }
//    return 0;
//}




//#if 1
//#include <sol/sol.hpp>
//#include <iostream>
//#include <chrono>
//
//int main() {
//    sol::state lua;
//    int x = 0;
//    lua.set_function("beep", [&x]{ return ++x; });
//
//    auto t = std::chrono::steady_clock::now();
//    lua.script(R"(
//for i = 1, 10000000 do
//    beep()
//end
//)");
//    std::cout << std::chrono::duration_cast<std::chrono::milliseconds>( std::chrono::steady_clock::now() - t ).count() << std::endl;
//    std::cout << x << std::endl;
//    t = std::chrono::steady_clock::now();
//
//    sol::function sf = lua["beep"];
//    std::function<int()> f = sf;
//    for (int i = 0; i < 10000000; ++i) {
//        f();
//    }
//    std::cout << std::chrono::duration_cast<std::chrono::milliseconds>( std::chrono::steady_clock::now() - t ).count() << std::endl;
//    std::cout << x << std::endl;
//}
//
//#else
//
//#include <xx_lua.h>
//#include <iostream>
//#include <chrono>
//
//namespace XL = xx::Lua;
//
//int main() {
//    xx::Lua::State L;
//    if (auto r = XL::Try(L, [&] {
//        int x = 0;
//        XL::SetGlobal(L, "beep", [&x] { return ++x; });
//
//        auto t = std::chrono::steady_clock::now();
//        luaL_dostring(L, R"(
//for i = 1, 10000000 do
//    beep()
//end
//)");
//        std::cout << std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now() - t).count() << std::endl;
//        std::cout << x << std::endl;
//        t = std::chrono::steady_clock::now();
//
//        std::function<int()> f;
//        XL::GetGlobal(L, "beep", f);
//        for (int i = 0; i < 10000000; ++i) {
//            f();
//        }
//        std::cout << std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now() - t).count() << std::endl;
//        std::cout << x << std::endl;
//    })) {
//        std::cout << "error! " << r.m << std::endl;
//    }
//}
//
//#endif