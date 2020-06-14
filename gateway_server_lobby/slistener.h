#pragma once
#include "xx_epoll.h"
#include "speer.h"
namespace EP = xx::Epoll;

// 继承默认监听器覆盖关键函数
struct SListener : EP::TcpListener<SPeer> {
    // 透传构造函数
    using EP::TcpListener<SPeer>::TcpListener;

    // 连接已建立, 搞事
    void OnAccept(std::shared_ptr<SPeer> const& p) override;
};
