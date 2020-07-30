#include "mylog.h"
#include "xx_chrono.h"

int test() {
    // 预热
    for (int i = 0; i < 500'000; ++i) {
        LOG_INFO("asdf ", i, " , ", 2.3, "asdfasdf");
    }
    // 等落盘
    while(__xxLogger.Busy()) {
        std::this_thread::sleep_for(std::chrono::milliseconds (1));
    }

    auto &&beginMS = xx::NowSteadyEpochMS();
    for (int i = 0; i < 500'000; ++i) {
        LOG_INFO("asdf ", i, " , ", 2.3, "asdfasdf");
    }
    std::cout << "elapsed ms = " << xx::NowSteadyEpochMS() - beginMS << std::endl;
    // 等落盘
    while(__xxLogger.Busy()) {
        std::this_thread::sleep_for(std::chrono::milliseconds (1));
    }
    std::cout << "write to file total elapsed ms = " << xx::NowSteadyEpochMS() - beginMS << std::endl;
    return 0;
}

int main() {
    return test();
//    __xxLogger.cfg.logLevel = (int)xx::LogLevels::TRACE;
//    LOG_INFO("asdf ", 1, " , ", 2.3, "asdfasdf");
//    LOG_WARN("asdf ", 1, " , ", 2.3, "asdfasdf");
//    LOG_ERROR("asdf ", 1, " , ", 2.3, "asdfasdf");
//    LOG_TRACE("asdf ", 1, " , ", 2.3, "asdfasdf");
//    LOG_DEBUG("asdf ", 1, " , ", 2.3, "asdfasdf");
    //std::cin.get();
    return 0;
}
