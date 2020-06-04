#pragma once
#include "peer.h"
namespace EP = xx::Epoll;

// 客户端 连进来产生的 peer
struct CPeer : Peer {
    // 自增编号, accept 时填充
    uint32_t clientId = 0xFFFFFFFFu;

    // 允许访问的 service peers 的 id 的白名单
    std::unordered_set<uint32_t> serviceIds;

    void SendCommand_Open(uint32_t const& serviceId) {
        this->SendCommand("open", serviceId);
    }

    void SendCommand_Close(uint32_t const& serviceId) {
        this->SendCommand("close", serviceId);
    }

    void OnReceivePackage(char* const& buf, std::size_t const& len) override;

    // 收到内部指令
    void OnReceiveCommand(char* const& buf, std::size_t const& len) override;

};
