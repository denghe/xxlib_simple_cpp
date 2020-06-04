#pragma once
#include "xx_epoll.h"
#include <unordered_map>
namespace EP = xx::Epoll;

// 预声明
struct Listener;
using Listener_r = EP::Ref<Listener>;
struct Dialer;
using Dialer_r = EP::Ref<Dialer>;
struct CPeer;
using CPeer_r = EP::Ref<CPeer>;
struct SPeer;
using SPeer_r = EP::Ref<SPeer>;

// 服务本体
struct Server : EP::Context {
    // 在构造函数中根据 config 进一步初始化各种拨号器
    explicit Server(size_t const &wheelLen = 1 << 12);

    // 析构当前类中的一些成员
    ~Server() override;

    // 帧逻辑：遍历 dialerPeers 检查 Peer 状态并自动拨号
    int FrameUpdate() override;

    // 等待客户端接入的监听器
    Listener_r listener;

    // 需要连接的服务器 拨号器 + 连接 字典. 加载配置后初始化
    std::unordered_map<int, std::pair<Dialer_r, SPeer_r>> dps;
};
