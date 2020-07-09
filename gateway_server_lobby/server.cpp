#include "server.h"
#include "gpeer.h"
#include "speer.h"
#include "config.h"
#include "glistener.h"
#include "slistener.h"

int Server::Run() {
    // 初始化回收sg, 以便退出 Run 时清理会加持宿主的成员
    xx::ScopeGuard sg1([&]{
        sps.clear();
        serverListener.reset();
        gps.clear();
        gatewayListener.reset();
        DisableCommandLine();
    });

    // 初始化监听器
    xx::MakeTo(gatewayListener, shared_from_this());
    if (int r = gatewayListener->Listen(config.gatewayListenPort)) {
        std::cout << "listen to port " <<config.gatewayListenPort << "failed." << std::endl;
        return r;
    }
    xx::MakeTo(serverListener, shared_from_this());
    if (int r = serverListener->Listen(config.serverListenPort)) {
        std::cout << "listen to port " <<config.serverListenPort << "failed." << std::endl;
        return r;
    }

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

    // 开始循环
    return this->EP::Context::Run();
}

int Server::FrameUpdate() {
    return 0;
}
