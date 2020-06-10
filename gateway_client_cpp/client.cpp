#include "client.h"
#include "dialer.h"
#include "config.h"
#include "peer.h"
#include "xx_chrono.h"
#include "coro.h"

Client::Client(size_t const &wheelLen) : EP::Context(wheelLen) {
    // 注册交互指令
    this->EnableCommandLine();

    // todo

    cmds["quit"] = [this](auto args) {
        running = false;
    };

    cmds["exit"] = cmds["quit"];
}

int Client::FrameUpdate() {
    lineNumber = Update();
    if (!lineNumber) return 1;
    return 0;
}

