#pragma once
#include "xx_epoll.h"
namespace EP = xx::Epoll;

// 预声明
struct Server;

// 继承默认监听器覆盖关键函数
struct Listener : EP::TcpListener {
    // 透传构造函数
    using EP::TcpListener::TcpListener;

    // 拿到 server 上下文引用, 以方便写事件处理代码
    Server &GetServer();

    // 提供创建目标类实例的内存操作支持
    EP::TcpPeer_u OnCreatePeer() override;

    // 连接已建立, 搞事
    void OnAccept(EP::TcpPeer_r const& peer_) override;
};
