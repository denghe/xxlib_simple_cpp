#include "server.h"
#include "dialer.h"
#include "speer.h"
#include "config.h"
#include "listener.h"

Server::Server(size_t const &wheelLen) : EP::Context(wheelLen) {
    // 初始化监听器
    listener = CreateTcpListener<Listener>(::config.listenPort);
    if (!this->listener) {
        throw -1;
    }

    // 遍历配置并生成相应的 dialer
    for (auto &&si : config.serverInfos) {
        // 创建拨号器
        auto dialer = CreateTcpDialer<Dialer>();
        if (!dialer) {
            throw -2;
        }
        // 放入字典。如果 server id 重复就报错
        if (dps.insert({si.serverId, std::make_pair(dialer, SPeer_r())}).second) {
            throw -3;
        }
        // 填充数据，为开始拨号作准备（会在帧回调逻辑中开始拨号）
        dialer->serverId = si.serverId;
        dialer->AddAddress(si.ip, si.port);
    }

    // 核查是否存在 0 号服务的 dialer. 没有就报错
    if (dps.find(0) == dps.end()) {
        throw -4;
    }
}

Server::~Server() {
    // Dispose 各种弱引用对象
    for (auto &&iter : dps) {
        auto &&dialer = iter.second.first;
        auto &&peer = iter.second.second;
        if (dialer) {
            dialer->Dispose();
        }
        if (peer) {
            peer->Dispose();
        }
    }
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
