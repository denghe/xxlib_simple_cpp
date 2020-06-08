#pragma once

#include "xx_epoll.h"
#include <unordered_map>

namespace EP = xx::Epoll;

// 预声明
struct GListener;
using GListener_r = EP::Ref<GListener>;
struct GPeer;
using GPeer_r = EP::Ref<GPeer>;
struct Dialer;
using Dialer_r = EP::Ref<Dialer>;
struct LPeer;
using LPeer_r = EP::Ref<LPeer>;

// 服务本体
struct Server : EP::Context {
    // 等待 gateway 接入的监听器
    GListener_r gatewayListener;

    // gateway peers
    std::unordered_map<uint32_t, GPeer_r> gps;

    // 连到 lobby 的拨号器
    Dialer_r lobbyDialer;

    // 保存 lobby 连接
    LPeer_r lobbyPeer;

    // 在构造函数中根据 config 进一步初始化各种拨号器
    Server(size_t const &wheelLen = 1 << 12);

    // 帧逻辑：遍历 dialerPeers 检查 Peer 状态并自动拨号
    int FrameUpdate() override;
};
