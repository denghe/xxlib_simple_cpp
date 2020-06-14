#pragma once
#include "xx_epoll.h"
#include "lpeer.h"
namespace EP = xx::Epoll;

// 预声明
struct Server;

// 连到 lobby 的拨号器
struct Dialer : EP::TcpDialer<LPeer> {
    // 透传构造函数
    using EP::TcpDialer<LPeer>::TcpDialer;

    // 拿到 server 上下文引用, 以方便写事件处理代码
    Server &GetServer();

    // 连接已建立, 继续与目标服务协商？
    void OnConnect(std::shared_ptr<LPeer> const &peer) override;
};
