#include "server.h"
#include "peer.h"
#include "gpeer.h"
#include "dialer.h"
#include "config.h"
#include "glistener.h"

int Server::Run() {
    // 初始化回收sg, 以便退出 Run 时清理会加持宿主的成员
    xx::ScopeGuard sg1([&]{
        DisableCommandLine();
        
        if(lobbyPeer) {
            lobbyPeer->Close(__LINE__, __FILE__);
            lobbyPeer.reset();
        }

        if (lobbyDialer) {
            lobbyDialer->Stop();
            lobbyDialer.reset();
        }

        std::vector<uint32_t> keys;
        for(auto&& gp : gps) {
            keys.push_back(gp.first);
        }
        for(auto&& key : keys) {
            gps[key]->Close(__LINE__, __FILE__);
        }
        gps.clear();

        gatewayListener.reset();
        holdItems.clear();
        auto c = shared_from_this().use_count();
        assert(c == 2);
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
        xx::CoutN("gps.size() = ", gps.size());
        xx::CoutN("gatewayId		ip:port");
        for (auto &&kv : gps) {
            xx::CoutN(kv.first, "\t\t", kv.second->addr);
        }
        xx::CoutN("dial to:		ip:port		busy		peer alive");
        xx::CoutN("lobby"
                  , "\t\t" , lobbyDialer->addrs[0]
                  , "\t\t" , (lobbyDialer->Busy() ? "true" : "false")
                  , "\t\t" , (lobbyPeer ? "true" : "false"));
    };

    cmds["quit"] = [this](auto args) {
        running = false;
    };
    cmds["exit"] = this->cmds["quit"];

    // 进入循环
    return this->EP::Context::Run();
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
