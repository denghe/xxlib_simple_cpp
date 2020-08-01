#pragma once
#include "xx_epoll.h"
#include <unordered_map>
namespace EP = xx::Epoll;

// 预声明
struct Listener;
struct Dialer;
struct CPeer;
struct SPeer;
struct PingTimer;
struct TaskTimer;

// 服务本体
struct Server : EP::Context {
    // 客户端连接id 自增量, 产生 peer 时++填充
    uint32_t cpeerAutoId = 0;

    // 等待 client 接入的监听器
    std::shared_ptr<Listener> listener;

    // 用于 ping 内部服务的 timer
    std::shared_ptr<PingTimer> pingTimer;

    // 用于计划任务的 timer
    std::shared_ptr<TaskTimer> taskTimer;

    // client peers
    std::unordered_map<uint32_t, std::shared_ptr<CPeer>> cps;

    // dialer + peer s
    std::unordered_map<uint32_t, std::pair<std::shared_ptr<Dialer>, std::shared_ptr<SPeer>>> dps;

    // 继承构造函数
    using EP::Context::Context;

    // 根据 config 进一步初始化各种成员. 并于退出时清理
    int Run() override;

    // 帧逻辑：遍历 dialerPeers 检查 Peer 状态并自动拨号
    int FrameUpdate() override;

    // 得到执行情况的快照
    std::string GetInfo();

    // 令日志输出到文件？
//    void LogN(int const& n, std::string&& txt) override;
};
