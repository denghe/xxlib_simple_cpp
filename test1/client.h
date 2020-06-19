#pragma once
#include "xx_uv.h"
#include "coro.h"
#include "resolver.h"
namespace UV = xx::Uv;

struct Client : UV::Context {
    using UV::Context::Context;

    // 模拟客户端帧逻辑. 每一帧驱动协程
    int FrameUpdate() override;

    // 所有正在执行的协程
    std::vector<std::unique_ptr<Coro>> coros;

    // 创建并添加协程
    template<typename CoroType>
    CoroType& CreateCoro() {
        auto&& coro = coros.emplace_back(std::make_unique<CoroType>(*this));
        return *(CoroType*)coro.get();
    }

};
