#include "server.h"
#include "dialer.h"
#include "config.h"
#include "listener.h"
#include "pingtimer.h"

int Server::Run() {
    // 初始化回收sg, 以便退出 Run 时清理会加持宿主的成员
    xx::ScopeGuard sg1([&]{
        DisableCommandLine();
        listener.reset();
        pingTimer.reset();

        for(auto&& dp : dps) {
            dp.second.first->Stop();
            if (dp.second.second) {
                dp.second.second->Close(__LINE__, __FILE__);
            }
        }
        dps.clear();

        std::vector<uint32_t> keys;
        for(auto&& cp : cps) {
            keys.push_back(cp.first);
        }
        for(auto&& key : keys) {
            cps[key]->Close(__LINE__, __FILE__);
        }
        cps.clear();

        holdItems.clear();

        assert(shared_from_this().use_count() == 2);
    });

    // 初始化监听器和回收sg
    xx::MakeTo(listener, shared_from_this());
    // 如果监听失败则输出错误提示并退出
    if (int r = listener->Listen(config.listenPort)) {
        Log<1>("listen to port ", config.listenPort, "failed.");
        return r;
    }

    // 初始化间隔时间为 1 秒的处理服务器之间 ping 防止连接僵死的 timer
    xx::MakeTo(pingTimer, shared_from_this());
    pingTimer->SetTimeoutSeconds(1);

    // 遍历配置并生成相应的 dialer
    for (auto &&si : config.serverInfos) {
        // 创建拨号器
        auto&& dialer = xx::Make<Dialer>(shared_from_this());
        // 放入字典。如果 server id 重复就报错
        if (!dps.insert({si.serverId, std::make_pair(dialer, nullptr)}).second) {
            Log<1>("duplicate serverId: ", si.serverId);
            return __LINE__;
        }
        // 填充数据，为开始拨号作准备（会在帧回调逻辑中开始拨号）
        dialer->serverId = si.serverId;
        dialer->AddAddress(si.ip, si.port);
    }

    // 核查是否存在 0 号服务的 dialer. 没有就报错
    if (dps.find(0) == dps.end()) {
        Log<1>("can't find base server ( serverId = 0 )'s dialer.");
        return __LINE__;
    }

    // 注册交互指令
    EnableCommandLine();

    cmds["cfg"] = [this](auto args) {
        std::cout << "cfg = " << config << std::endl;
    };
    cmds["info"] = [this](auto args) {
        std::cout << GetInfo() << std::endl;
    };
    cmds["quit"] = [this](auto args) {
        running = false;
    };
    cmds["exit"] = this->cmds["quit"];

    // 正式进入 epoll wait 循环
    return this->EP::Context::Run();
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

std::string Server::GetInfo() {
    std::string s;
    xx::Append(s, "dps.size() = ", dps.size()\
    , "\r\nserverId		ip:port		busy		peer alive\r\n");
    for (auto &&kv : dps) {
        auto &&dialer = kv.second.first;
        auto &&peer = kv.second.second;
        xx::Append(s, dialer->serverId
                 , "\t\t", xx::ToString(dialer->addrs[0])
                 , "\t\t", (dialer->Busy() ? "true" : "false")
                 , "\t\t", (peer ? "true" : "false"), "\r\n");
    }
    xx::Append(s, "cps.size() = ", cps.size()
            , "\r\nclientId		ip:port\r\n");
    for (auto &&kv : cps) {
        xx::Append(s, kv.first, kv.second->addr);
    }

    return s;
}

//void Server::LogN(int const& n, std::string&& txt) {
// todo
//}
