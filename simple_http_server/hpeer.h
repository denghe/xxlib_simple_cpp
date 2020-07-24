#pragma once
#include "xx_epoll_http.h"
#include "xx_string.h"

namespace EP = xx::Epoll;

// 继承 默认 连接覆盖收包函数
struct HPeer : EP::HttpPeerEx {
    using EP::HttpPeerEx::HttpPeerEx;

    bool Close(int const &reason, char const* const& desc) override;
    ~HPeer();
    void ReceiveHttp() override;
};
