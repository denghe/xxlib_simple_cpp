#include "client.h"
#include "dialer.h"
#include "config.h"
#include "peer.h"
#include "xx_chrono.h"
#include "coromain.h"

bool Client::ExistsCoroName(std::string const& name) {
    return coroNames.find(name) != coroNames.end();
}

int Client::Run(double const &frameRate) {
    // 创建回收器
    xx::ScopeGuard sgCmdLine([&] {
        // 反注册交互指令( 消除对 Context 的引用计数的影响 )
        DisableCommandLine();
        // 清理成员变量( 消除对 Context 的引用计数的影响 )
        dialer.reset();
        peer.reset();
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
    return this->EP::Context::Run(frameRate);
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
