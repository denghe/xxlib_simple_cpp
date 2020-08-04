#include "xx_chrono.h"
#include "xx_epoll.h"
#include "xx_logger.h"

int test() {
    // 预热
    for (int i = 0; i < 500'000; ++i) {
        LOG_INFO("asdf ", i, " , ", 2.3, "asdfasdf");
    }
    // 等落盘
    while (__xxLogger.Busy()) {
        std::this_thread::sleep_for(std::chrono::milliseconds(1));
    }

    auto &&beginMS = xx::NowSteadyEpochMS();
    for (int i = 0; i < 500'000; ++i) {
        LOG_INFO("asdf ", i, " , ", 2.3, "asdfasdf");
    }
    std::cout << "elapsed ms = " << xx::NowSteadyEpochMS() - beginMS << std::endl;
    // 等落盘
    while (__xxLogger.Busy()) {
        std::this_thread::sleep_for(std::chrono::milliseconds(1));
    }
    std::cout << "write to file total elapsed ms = " << xx::NowSteadyEpochMS() - beginMS << std::endl;
    return 0;
}

struct XXX {
    void xxx() {
        LOG_INFO("asdf ");
    }
};

int main() {
    //return test();
    sockaddr_in6 asdf{};
    xx::Epoll::FillAddress("192.168.1.1", 123, asdf);

    xx::Data d;
    d.WriteFixed(1);
    d.WriteFixed(2);
    xx::DataView dv{d.buf, d.len-4};
    LOG_INFO("asdf ", 1, " , ", 2.3, "asdfasdf ", asdf, " 123 ", d, "  asdf", dv, " !!!");
//    LOG_WARN("asdf ", 1, " , ", 2.3, "asdfasdf");
//    LOG_ERROR("asdf ", 1, " , ", 2.3, "asdfasdf");
//    LOG_TRACE("asdf ", 1, " , ", 2.3, "asdfasdf");
//    LOG_DEBUG("asdf ", 1, " , ", 2.3, "asdfasdf");
    //std::cin.get();

    ((XXX*)0)->xxx();

    return 0;
}
