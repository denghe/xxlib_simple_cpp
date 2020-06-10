#pragma once

#include "xx_epoll.h"
#include "package.h"
#include <unordered_map>
#include <deque>

// todo: 连接网关走完整流程

namespace EP = xx::Epoll;

// 预声明
struct Dialer;
using Dialer_r = EP::Ref<Dialer>;

struct Peer;
using Peer_r = EP::Ref<Peer>;


// 服务本体
struct Client : EP::Context {
    // 在构造函数中根据 config 进一步初始化
    Client(size_t const &wheelLen = 1 << 12);

    // 模拟客户端帧逻辑
    int FrameUpdate() override;


    // 协程模拟
    int Update();

    // 协程模拟之 行号 内部变量
    int lineNumber = 0;

    // 下面是协程中要用到的用户变量

    // 到网关的拨号器
    Dialer_r dialer;

    // 拨号器连接成功后将 peer 存在此以便使用
    Peer_r peer;

    // 存当前时间
    double nowSecs = 0;

    // 通常用于存放返回值
    int r = 0;

    // 某阶段状态时存储需要与之通信的 分类存放的 服务 id. 按类型互斥
    uint32_t lobbyServerId = -1;
    uint32_t gameServerId = -1;

    // 按 serverId 分类存放的消息队列
    std::unordered_map<uint32_t, std::deque<Package>> serverPackages;
};
