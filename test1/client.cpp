#include "client.h"

int Client::FrameUpdate() {
    // 倒着扫, 便于中途交焕移除
    for (auto &&i = coros.size() - 1; i != (size_t) -1; --i) {
        // 拿到协程指针
        auto &&coro = coros[i].get();
        // 执行协程
        coro->lineNumber = coro->Update();
        // 如果协程已执行完毕
        if (!coro->lineNumber) {
            // 如果不是最后一个, 就和最后一个交焕位置
            if(i < coros.size() - 1) {
                std::swap(coros[coros.size() - 1], coros[i]);
            }
            // 移除 coros 最后一个
            coros.pop_back();
        }
    }
    // 如果一个不剩，通知程序退出
    return coros.empty() ? 1 : 0;
}
