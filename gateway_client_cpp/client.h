#pragma once

#include "xx_epoll.h"

// todo: 连接网关走完整流程

namespace EP = xx::Epoll;

// 预声明
struct Dialer;
using Dialer_r = EP::Ref<Dialer>;

struct Peer;
using Peer_r = EP::Ref<Peer>;


// 服务本体
struct Client : EP::Context {
    // 到网关的拨号器
    Dialer_r dialer;

    // 拨号器连接成功后将 peer 存在此以便使用
    Peer_r peer;

    // 初始化拨号器
    void InitDialer();
    void CleanupDialerAndPeer();

    int lineNumber = 0;
    int Update();

    // 模拟一个客户端帧逻辑
    int FrameUpdate() override;

    // 在构造函数中根据 config 进一步初始化
    Client(size_t const &wheelLen = 1 << 12);
};
