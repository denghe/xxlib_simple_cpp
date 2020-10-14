﻿#include "xx_signal.h"
#include "server.h"
#include "generator.h"

xx::Generator<int> Delay(int n) {
    for (int i = 0; i < n; ++i) {
        std::cout << i << std::endl;
        co_yield 0;
    }
}

int main() {
    Delay(5).AWait();
    return 0;

//    GetInts2().Foreach([](auto&& i){
//        std::cout << i << std::endl;
//    });
//    return 0;

    // 禁掉 SIGPIPE 信号避免因为连接关闭出错
    xx::IgnoreSignal();

    // 创建类实例. 轮长 = 最大超时秒数 * 帧数 的 2^n 对齐. kcp 需要每秒 100 帧
    auto &&s = xx::Make<Server>(32768, 100);

    // 开始运行
    return s->Run();
}

//xx::Generator<int> GetInts() {
//    for (int i = 0; i < 5; ++i) {
//        co_yield i;
//    }
//}
//
//xx::Generator<int> GetInts2() {
//    auto g = GetInts();
//    for (int i = 0; i < 10; ++i) {
//        if (g.Next()) co_yield g.Value();
//        else g = GetInts();
//    }
//}