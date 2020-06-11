#pragma once
#include "coro.h"
struct CoroLobby : Coro {
    // 透传继承构造函数
    using Coro::Coro;

    // 提供协程名
    std::string name = "lobby";
    inline std::string const& GetName() override { return name; }

    // 自己的协程逻辑
    int Update() override;

    // 存当前时间
    double nowSecs = 0;

    // 通常用于存放返回值
    int r = 0;
};
