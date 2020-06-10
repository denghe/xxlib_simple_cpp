#pragma once
#include "coro.h"
struct Coro1 : Coro {
    int Update() override;

    // 存当前时间
    double nowSecs = 0;

    // 通常用于存放返回值
    int r = 0;
};
