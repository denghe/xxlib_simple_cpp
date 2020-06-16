#pragma once
#include "xx_epoll.h"
#include "peer.h"
namespace EP = xx::Epoll;

// 预声明
struct Client;

// 连到 lobby, game servers 的拨号器
struct Dialer : EP::TcpDialer<Peer> {
    // 透传构造函数
    using EP::TcpDialer<Peer>::TcpDialer;

    // 拿到 server 上下文引用, 以方便写事件处理代码
    Client &GetClient();

    // 连接已建立, 继续与目标服务协商？
    void Connect(std::shared_ptr<Peer> const &peer) override;
};
