#include "server.h"
#include "peer.h"
#include "config.h"
#include "listener.h"

Server::Server(size_t const &wheelLen) : EP::Context(wheelLen) {
    // 初始化监听器
    listener = CreateTcpListener<Listener>(config.listenPort);
    if (!this->listener) {
        throw std::logic_error(std::string("listen to port: ") + std::to_string(config.listenPort) + " failed.");
    }

    // 注册交互指令
    EnableCommandLine();

    cmds["cfg"] = [this](auto args) {
        std::cout << "cfg = " << config << std::endl;
    };
    cmds["info"] = [this](auto args) {
        std::cout << "aps.size() = " << aps.size() << std::endl;
        std::cout << "gps.size() = " << gps.size() << std::endl;
        std::cout << "gatewayId		ip:port" << std::endl;
        for (auto &&kv : gps) {
            std::cout << kv.first << "\t\t" << EP::AddressToString(kv.second->addr) << std::endl;
        }
        std::cout << "sps.size() = " << sps.size() << std::endl;
        std::cout << "serverId		ip:port" << std::endl;
        for (auto &&kv : sps) {
            std::cout << kv.first << "\t\t" << EP::AddressToString(kv.second->addr) << std::endl;
        }
    };

    cmds["quit"] = [this](auto args) {
        running = false;
    };
    cmds["exit"] = this->cmds["quit"];
}

int Server::FrameUpdate() {
    return 0;
}
