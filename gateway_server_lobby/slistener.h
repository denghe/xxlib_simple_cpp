#pragma once
#include "xx_epoll.h"
namespace EP = xx::Epoll;

// 继承默认监听器覆盖关键函数
struct SListener : EP::TcpListener {
    // 透传构造函数
    using EP::TcpListener::TcpListener;

    // 提供创建目标类实例的内存操作支持
    EP::TcpPeer_u OnCreatePeer() override;

    // 连接已建立, 搞事
    void OnAccept(EP::TcpPeer_r const& p) override;
};
