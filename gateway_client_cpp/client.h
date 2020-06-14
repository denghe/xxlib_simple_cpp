#pragma once

#include "xx_epoll.h"
#include "package.h"
#include <unordered_map>
#include <deque>
#include <string>
#include <unordered_set>
#include "coro.h"

// todo: 连接网关走完整流程

namespace EP = xx::Epoll;

// 预声明
struct Dialer;
struct Peer;

// 服务本体
struct Client : EP::Context {
    // 继承构造函数
    using EP::Context::Context;

    // 根据 config 进一步初始化. 创建主线协程
    int Run(double const &frameRate) override;

    // 模拟客户端帧逻辑. 每一帧驱动协程
    int FrameUpdate() override;

    // 所有正在执行的协程
    std::vector<std::unique_ptr<Coro>> coros;

    // 所有正在执行的协程 的 名字集合( 方便判断是否存在某协程正在执行 )
    std::unordered_set<std::string> coroNames;

    // return coroNames.find(name) != coroNames.end()
    bool ExistsCoroName(std::string const& name);

    // 创建并添加协程
    template<typename CoroType>
    CoroType& CreateCoro() {
        auto&& coro = coros.emplace_back(std::make_unique<CoroType>(*this));
        coroNames.emplace(coro->GetName());
        return *(CoroType*)coro.get();
    }

    // 到网关的拨号器
    std::shared_ptr<Dialer> dialer;

    // 拨号器连接成功后将 peer 存在此以便使用
    std::shared_ptr<Peer> peer;

    // 某阶段状态时存储需要与之通信的 分类存放的 服务 id. 按类型互斥。同时也是相关 coro 是否继续运行的判断依据
    uint32_t lobbyServerId = -1;
    uint32_t gameServerId = -1;

    // 按 serverId 分类存放的消息队列
    std::unordered_map<uint32_t, std::deque<Package>> serverPackages;
};
