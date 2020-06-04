#pragma once
#include "peer.h"
namespace EP = xx::Epoll;

struct CPeer;

// 拨号到服务器 产生的 peer
struct SPeer : Peer {
    // 内部服务编号, 从配置填充
    uint32_t serverId = 0xFFFFFFFFu;

    // 发送自己的 gateway id 给 peer 所连服务
    void SendCommand_GatewayId(uint32_t const& gatewayId);

    // 客户端连上来后发送 client + ip 给 peer 所连服务
    void SendCommand_Accept(uint32_t const& clientId, std::string const& ip);

    // 通知 peer 所连服务, 某 client 已断开
    void SendCommand_Disconnect(uint32_t const& clientId);

    // 收到正常包
    void OnReceivePackage(char* const& buf, size_t const& len) override;

    // 收到内部指令
    void OnReceiveCommand(char* const& buf, size_t const& len) override;

    // 断开时 清除所有 client peer 中的 相关 open id. 列表被清空则踢掉
    void OnDisconnect(int const &reason) override;

    // 从 server cps 容器查找并返回 client peer 的指针. 没找到则返回 空
    CPeer* TryGetCPeer(uint32_t const& clientId);
};
