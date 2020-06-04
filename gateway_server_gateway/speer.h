#pragma once
#include "peer.h"
namespace EP = xx::Epoll;

// 拨号到服务器 产生的 peer
struct SPeer : Peer {
    // 内部服务编号, 从配置填充
    uint32_t serverId = 0xFFFFFFFFu;

    // 等待 ping 回包的状态值
    bool waitPingBack = false;

    void SendCommand_GatewayId(uint32_t const& gatewayId) {
        this->SendCommand("gatewayId", gatewayId);
    }

    void SendCommand_Accept(uint32_t const& clientId, std::string const& ip) {
        this->SendCommand("accept", clientId, ip);
    }

    void SendCommand_Disconnect(uint32_t const& clientId) {
        this->SendCommand("disconnect", clientId);
    }
    void SendCommand_Ping() {
        this->SendCommand("ping", xx::NowEpoch10m());
    }

    void OnReceivePackage(char* const& buf, std::size_t const& len) override;

    // 收到内部指令
    void OnReceiveCommand(char* const& buf, std::size_t const& len) override;
};
