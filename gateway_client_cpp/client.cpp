﻿#include "client.h"
#include "dialer.h"
#include "config.h"
#include "xx_chrono.h"
#include "coromain.h"

bool Client::ExistsCoroName(std::string const& name) {
    return coroNames.find(name) != coroNames.end();
}

int Client::Run() {
    // 初始化回收sg, 以便退出 Run 时清理会加持宿主的成员
    xx::ScopeGuard sg([&] {
        DisableCommandLine();
        dialer.reset();
        if(peer) {
            peer->Close(__LINE__);
            peer.reset();
        }
        holdItems.clear();
        assert(shared_from_this().use_count() == 2);
    });
    // 创建默认协程
    CreateCoro<CoroMain>();

    // 注册交互指令
    EnableCommandLine();
    // todo
    cmds["quit"] = [this](auto args) {
        running = false;
    };
    cmds["exit"] = cmds["quit"];

    // 开始循环
    return this->EP::Context::Run();
}

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
