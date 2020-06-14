#include "server.h"
#include "peer.h"
#include "gpeer.h"
#include "dialer.h"
#include "config.h"
#include "glistener.h"

int Server::Run(double const &frameRate) {
    // 初始化回收sg
    xx::ScopeGuard sg1([&]{
        gatewayListener.reset();
        lobbyDialer.reset();
        DisableCommandLine();
    });
    // 创建监听器
    xx::MakeTo(gatewayListener, shared_from_this());
    if (int r = gatewayListener->Listen(config.gatewayListenPort)) {
        std::cout << "listen to port " <<config.gatewayListenPort << "failed." << std::endl;
        return r;
    }

    // 创建拨号器
    xx::MakeTo(lobbyDialer, shared_from_this());
    // 添加拨号地址
    lobbyDialer->AddAddress(config.lobbyIp, config.lobbyPort);

    // 注册交互指令
    EnableCommandLine();

    cmds["cfg"] = [this](auto args) {
        std::cout << "cfg = " << config << std::endl;
    };
    cmds["info"] = [this](auto args) {
        std::cout << "gps.size() = " << gps.size() << std::endl;
        std::cout << "gatewayId		ip:port" << std::endl;
        for (auto &&kv : gps) {
            std::cout << kv.first << "\t\t" << EP::AddressToString(kv.second->addr) << std::endl;
        }
        std::cout << "dial to:		ip:port		busy		peer alive" << std::endl;
        std::cout << "lobby"
                  << "\t\t" << EP::AddressToString(lobbyDialer->addrs[0])
                  << "\t\t" << (lobbyDialer->Busy() ? "true" : "false")
                  << "\t\t" << (lobbyPeer ? "true" : "false") << std::endl;
    };

    cmds["quit"] = [this](auto args) {
        running = false;
    };
    cmds["exit"] = this->cmds["quit"];

    // 进入循环
    return this->EP::Context::Run(frameRate);
}

int Server::FrameUpdate() {
    // 自动拨号 & 重连逻辑
    // 未建立连接
    if (!lobbyPeer) {
        // 并非正在拨号
        if (!lobbyDialer->Busy()) {
            // 超时时间 2 秒
            lobbyDialer->DialSeconds(2);
        }
    }
    return 0;
}
