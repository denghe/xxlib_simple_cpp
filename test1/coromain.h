#pragma once
#include "coro.h"
#include "resolver.h"
#include "listener.h"
struct CoroMain : Coro {
    // 透传继承构造函数
    using Coro::Coro;

    // 自己的协程逻辑
    int Update() override;

    // 存当前时间
    double nowSecs = 0;

    std::shared_ptr<Resolver> resolver;
    std::shared_ptr<Listener> listener;
};
