#include "server.h"
#include "dialer.h"
#include "config.h"
#include "listener.h"

int Server::Run(double const &frameRate) {
    // 初始化回收sg
    xx::ScopeGuard sg1([&]{
        listener.reset();
        dps.clear();
        DisableCommandLine();
    });

    // 初始化监听器和回收sg
    xx::MakeTo(listener, shared_from_this());
    // 如果监听失败则输出错误提示并退出
    if (int r = listener->Listen(config.listenPort)) {
        std::cout << "listen to port " <<config.listenPort << "failed." << std::endl;
        return r;
    }

    // 遍历配置并生成相应的 dialer
    for (auto &&si : config.serverInfos) {
        // 创建拨号器
        auto&& dialer = xx::Make<Dialer>(shared_from_this());
        // 放入字典。如果 server id 重复就报错
        if (!dps.insert({si.serverId, std::make_pair(dialer, nullptr)}).second) {
            std::cout << "duplicate serverId: " << si.serverId << std::endl;
            return __LINE__;
        }
        // 填充数据，为开始拨号作准备（会在帧回调逻辑中开始拨号）
        dialer->serverId = si.serverId;
        dialer->AddAddress(si.ip, si.port);
    }

    // 核查是否存在 0 号服务的 dialer. 没有就报错
    if (dps.find(0) == dps.end()) {
        std::cout << "can't find base server ( serverId = 0 )'s dialer." << std::endl;
        return __LINE__;
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

    // 正式进入 epoll wait 循环
    return this->EP::Context::Run(frameRate);
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
