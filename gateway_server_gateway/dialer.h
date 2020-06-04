#pragma once
#include "xx_epoll.h"
namespace EP = xx::Epoll;

// 预声明
struct Server;

// 继承默认监听器覆盖关键函数
struct Dialer : EP::TcpDialer {
    // 透传构造函数
    using EP::TcpDialer::TcpDialer;

    // 备查
    int serverId = -1;

    // 拿到 server 上下文引用, 以方便写事件处理代码
    Server &GetServer();

    // 提供创建目标类实例的内存操作支持
    EP::TcpPeer_u OnCreatePeer() override;

    // 连接已建立, 继续与目标服务协商？
    void OnConnect(EP::TcpPeer_r const &peer_) override;
};
