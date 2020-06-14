#pragma once
#include "xx_epoll.h"
#include "peer.h"

namespace EP = xx::Epoll;

// 继承默认监听器覆盖关键函数
struct Dialer : EP::TcpDialer<Peer> {
    using EP::TcpDialer<Peer>::TcpDialer;

    // 连接已建立, 搞事
    void OnConnect(std::shared_ptr<Peer> const &peer) override;
};
