#pragma once
#include "xx_epoll.h"
namespace EP = xx::Epoll;

struct PingTimer : EP::Timer {
    using EP::Timer::Timer;
    void Timeout() override;
};
