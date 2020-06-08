#include "server.h"
#include "dialer.h"
#include "speer.h"
#include "cpeer.h"
#include "config.h"
#include "listener.h"

Server::Server(size_t const &wheelLen) : EP::Context(wheelLen) {
    // 初始化监听器
    listener = CreateTcpListener<Listener>(::config.listenPort);
    if (!this->listener) {
        throw std::logic_error(std::string("listen to port ") + std::to_string(config.listenPort) + "failed.");
    }

    // 遍历配置并生成相应的 dialer
    for (auto &&si : config.serverInfos) {
        // 创建拨号器
        auto dialer = CreateTcpDialer<Dialer>();
        if (!dialer) {
            throw std::logic_error("create dialer failed.");
        }
        // 放入字典。如果 server id 重复就报错
        if (!dps.insert({si.serverId, std::make_pair(dialer, SPeer_r())}).second) {
            throw std::logic_error(std::string("duplicate serverId: ") + std::to_string(si.serverId));
        }
        // 填充数据，为开始拨号作准备（会在帧回调逻辑中开始拨号）
        dialer->serverId = si.serverId;
        dialer->AddAddress(si.ip, si.port);
    }

    // 核查是否存在 0 号服务的 dialer. 没有就报错
    if (dps.find(0) == dps.end()) {
        throw std::logic_error("can't find base server ( serverId = 0 )'s dialer.");
    }


    // 注册交互指令
    EnableCommandLine();

    cmds["cfg"] = [this](auto args) {
        std::cout << "cfg = " << config << std::endl;
    };
    cmds["info"] = [this](auto args) {
        std::cout << "dps.size() = " << dps.size() << std::endl;
        std::cout << "serverId		ip:port		busy		peer alive" << std::endl;
        for (auto &&kv : dps) {
            auto &&dialer = kv.second.first;
            auto &&peer = kv.second.second;
            std::cout << dialer->serverId
                      << "\t\t" << EP::AddressToString(dialer->addrs[0])
                      << "\t\t" << (dialer->Busy() ? "true" : "false")
                      << "\t\t" << (peer ? "true" : "false") << std::endl;
        }
        std::cout << "cps.size() = " << cps.size() << std::endl;
        std::cout << "clientId		ip:port" << std::endl;
        for (auto &&kv : cps) {
            std::cout << kv.first
                      << EP::AddressToString(kv.second->addr) << std::endl;
        }
    };
    cmds["quit"] = [this](auto args) {
        running = false;
    };
    cmds["exit"] = this->cmds["quit"];
}

int Server::FrameUpdate() {
    // 自动拨号 & 重连逻辑
    for (auto &&iter : dps) {
        auto &&dialer = iter.second.first;
        auto &&peer = iter.second.second;
        // 未建立连接
        if (!peer) {
            // 并非正在拨号
            if (!dialer->Busy()) {
                // 超时时间 2 秒
                dialer->DialSeconds(2);
            }
        }
    }
    return 0;
}
