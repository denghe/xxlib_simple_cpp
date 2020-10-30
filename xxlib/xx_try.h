#pragma once

#include <iostream>

namespace xx {
    template<typename F>
    void Try(F&& f) {
        try {
            f();
        }
        catch (std::exception const& ex) {
            std::cout << "throw ex.what() = " << ex.what() << std::endl;
        }
        catch (...) {
            std::cout << "throw ..." << std::endl;
        }
    }
}
