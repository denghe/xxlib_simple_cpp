#pragma once

#include "xx_epoll.h"

namespace EP = xx::Epoll;

// 预声明
struct Dialer;
struct Peer;

// 服务本体
struct Client : EP::Context {
    using EP::Context::Context;

    // 进入时初始化下面的类成员( 构造函数里做不了, 因为无法 shared_from_this ). 退出时清理这些类成员( 去除引用计数 )
    int Run() override;

    // 在帧逻辑里自动拨号到服务器
    int FrameUpdate() override;

    // 拨号器
    std::shared_ptr<Dialer> dialer;

    // 拨号器连接成功后将 peer 存在此以便使用
    std::shared_ptr<Peer> peer;
};
