#pragma once
#include "xx_epoll_kcp.h"
#include <unordered_map>
namespace EP = xx::Epoll;

// 预声明
struct Listener;
struct CPeer;

// 服务本体
struct Server : EP::Context {
    using EP::Context::Context;

    // 等待 client 接入的监听器
    std::shared_ptr<Listener> listener;

    // 进一步初始化各种成员. 并于退出时清理
    int Run() override;

    // 帧逻辑
    int FrameUpdate() override;
};
