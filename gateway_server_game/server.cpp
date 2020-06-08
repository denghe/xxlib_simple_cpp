#include "server.h"
#include "peer.h"
#include "dialer.h"
#include "config.h"
#include "listener.h"

Server::Server(size_t const &wheelLen) : EP::Context(wheelLen) {
    // 创建监听器
    listener = CreateTcpListener<Listener>(config.listenPort);
    if (!this->listener) {
        throw std::logic_error(std::string("listen to port: ") + std::to_string(config.listenPort) + " failed.");
    }

    // 创建拨号器
    dialer = CreateTcpDialer<Dialer>();
    if (!dialer) {
        throw std::logic_error("create dialer failed.");
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
        std::cout << "dial to:		ip:port		busy		peer alive" << std::endl;
        std::cout << "lobby"
                  << "\t\t" << EP::AddressToString(dialer->addrs[0])
                  << "\t\t" << (dialer->Busy() ? "true" : "false")
                  << "\t\t" << (lobbyPeer ? "true" : "false") << std::endl;
    };

    cmds["quit"] = [this](auto args) {
        running = false;
    };
    cmds["exit"] = this->cmds["quit"];
}

int Server::FrameUpdate() {
    // 自动拨号 & 重连逻辑
    // 未建立连接
    if (!lobbyPeer) {
        // 并非正在拨号
        if (!dialer->Busy()) {
            // 超时时间 2 秒
            dialer->DialSeconds(2);
        }
    }
    return 0;
}
