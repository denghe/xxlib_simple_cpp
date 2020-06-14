#pragma once
#include "xx_epoll.h"
#include "gpeer.h"
namespace EP = xx::Epoll;

// 继承默认监听器覆盖关键函数
struct GListener : EP::TcpListener<GPeer> {
    // 透传构造函数
    using EP::TcpListener<GPeer> ::TcpListener;

    // 连接已建立, 搞事
    void OnAccept(std::shared_ptr<GPeer> const& peer) override;
};
