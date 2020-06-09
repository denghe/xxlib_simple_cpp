#include "client.h"
#include "dialer.h"
#include "config.h"
#include "peer.h"

Client::Client(size_t const &wheelLen) : EP::Context(wheelLen) {
    // 注册交互指令
    this->EnableCommandLine();

    // todo

    cmds["quit"] = [this](auto args) {
        running = false;
    };

    cmds["exit"] = cmds["quit"];
}

void Client::InitDialer() {
    // 初始化拨号器
    dialer = CreateTcpDialer<Dialer>();
    if (!dialer) {
        throw -1;
    }

    // 添加拨号地址
    for (auto &&addr : config.addrs) {
        dialer->AddAddress(addr.ip, addr.port);
    }
}

void Client::CleanupDialerAndPeer() {
    dialer->Stop();
    if (peer) {
        peer->Dispose();
    }
}

int Client::FrameUpdate() {
    lineNumber = Update();
    if (!lineNumber) return 1;
    return 0;
}

#define COR_BEGIN    switch (lineNumber) { case 0:
#define COR_YIELD    return __LINE__; case __LINE__:;
#define COR_EXIT    return 0;
#define COR_END        } return 0;

int Client::Update() {
    COR_BEGIN {
            // 初始化拨号器
            InitDialer();

            LabDial:

            CleanupDialerAndPeer();
            dialer->DialSeconds(2);

        };
    COR_END
}
