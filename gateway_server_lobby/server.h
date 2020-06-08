#pragma once

#include "xx_epoll.h"
#include <unordered_map>

namespace EP = xx::Epoll;

// 预声明
struct GListener;
using GListener_r = EP::Ref<GListener>;
struct GPeer;
using GPeer_r = EP::Ref<GPeer>;

struct SListener;
using SListener_r = EP::Ref<SListener>;
struct SPeer;
using SPeer_r = EP::Ref<SPeer>;

// 服务本体
struct Server : EP::Context {
    // anonymous peers's auto increase id
    uint32_t autoIncId = 0;

    // 等待 gateway 接入的监听器
    GListener_r gatewayListener;

    // gateway: anonymous gateway peers
    std::unordered_map<uint32_t, GPeer_r> gaps;

    // gateway peers
    std::unordered_map<uint32_t, GPeer_r> gps;

    // 等待 server 接入的监听器
    SListener_r serverListener;

    // server: anonymous server peers
    std::unordered_map<uint32_t, SPeer_r> saps;

    // server peers
    std::unordered_map<uint32_t, SPeer_r> sps;

    // 在构造函数中根据 config 进一步初始化各种拨号器
    Server(size_t const &wheelLen = 1 << 12);

    // 帧逻辑：遍历 dialerPeers 检查 Peer 状态并自动拨号
    int FrameUpdate() override;
};
