#pragma once
#include "peer.h"
namespace EP = xx::Epoll;

// 预声明
struct SPeer;

// 客户端 连进来产生的 peer
struct CPeer : Peer {
    // 自增编号, accept 时填充
    uint32_t clientId = 0xFFFFFFFFu;

    // 允许访问的 server peers 的 id 的白名单
    std::unordered_set<uint32_t> serverIds;

    void SendCommand_Open(uint32_t const& serverId);

    void SendCommand_Close(uint32_t const& serverId);

    // 收到正常包
    void OnReceivePackage(char* const& buf, size_t const& len) override;

    // 收到内部指令
    void OnReceiveCommand(char* const& buf, size_t const& len) override;

    // 断开时 向 serverIds 对应的 server peer 群发断开指令
    void OnDisconnect(int const &reason) override;

    // 通过 server id 获取 相应的 服务器 peer 指针. 没找到 或者 已断开 返回 空
    SPeer *TryGetSPeer(uint32_t const &serverId);
};
