#include "mylog.h"
#include "xx_chrono.h"

int main() {
    // 预热
    for (int i = 0; i < 500'000; ++i) {
        LOG_INFO("asdf ", i, 2.3, "asdfasdf");
    }
    // 等落盘
    std::this_thread::sleep_for(std::chrono::seconds (2));

    auto &&beginMS = xx::NowSteadyEpochMS();
    for (int i = 0; i < 500'000; ++i) {
        LOG_INFO("asdf ", i, 2.3, "asdfasdf");
    }
    std::cout << "elapsed ms = " << xx::NowSteadyEpochMS() - beginMS << std::endl;
    return 0;
}
