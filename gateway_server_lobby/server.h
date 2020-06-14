#pragma once

#include "xx_epoll.h"
#include <unordered_map>

namespace EP = xx::Epoll;

// 预声明
struct GListener;
struct GPeer;

struct SListener;
struct SPeer;

// 服务本体
struct Server : EP::Context {
    // 等待 gateway 接入的监听器
    std::shared_ptr<GListener> gatewayListener;

    // gateway peers
    std::unordered_map<uint32_t, std::shared_ptr<GPeer>> gps;

    // 等待 server 接入的监听器
    std::shared_ptr<SListener> serverListener;

    // server peers
    std::unordered_map<uint32_t, std::shared_ptr<SPeer>> sps;

    // 继承构造函数
    using EP::Context::Context;

    // 根据 config 进一步初始化各种成员. 并于退出时清理
    int Run(double const &frameRate) override;

    // 帧逻辑
    int FrameUpdate() override;
};
