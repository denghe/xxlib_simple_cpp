#include "server.h"
#include "peer.h"
#include "config.h"
#include "listener.h"

Server::Server(size_t const &wheelLen) : EP::Context(wheelLen) {
    // 初始化监听器
    listener = CreateTcpListener<Listener>(::config.listenPort);
    if (!this->listener) {
        throw -1;
    }

    // 注册交互指令
    EnableCommandLine();

    cmds["cfg"] = [this](auto args) {
        std::cout << "cfg = " << config << std::endl;
    };
    cmds["info"] = [this](auto args) {
        std::cout << "ps.size() = " << ps.size() << std::endl;
        std::cout << "serverId		ip:port" << std::endl;
        for (auto &&kv : ps) {
            std::cout << kv.first
                      << EP::AddressToString(kv.second->addr) << std::endl;
        }
    };

    cmds["quit"] = [this](auto args) {
        running = false;
    };
    cmds["exit"] = this->cmds["quit"];
}

Server::~Server() {
    // Dispose 各种弱引用对象
    for (auto &&iter : ps) {
        if (iter.second) {
            iter.second->Dispose();
        }
    }
}

int Server::FrameUpdate() {
    return 0;
}
