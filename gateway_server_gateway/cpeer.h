#pragma once
#include "peer.h"

// 客户端 连进来产生的 peer
struct CPeer : Peer {
    // 自增编号, accept 时填充
    uint32_t clientId = 0xFFFFFFFFu;

    // 允许访问的 server peers 的 id 的白名单
    std::unordered_set<uint32_t> serverIds;

    // 继承构造函数
    using Peer::Peer;

    // 向 serverIds 对应的 server peer 群发断开指令, 延迟自杀
    bool Close(int const& reason) override;

    // 延迟关闭( 设置 closed = true, 立即触发 OnDisconnect, 设置超时, 从容器移除并 hold. 令 clientId = 0xFFFFFFFFu )
    void DelayClose(double const& delaySeconds);

    // 收到正常包
    void ReceivePackage(char* const& buf, size_t const& len) override;

    // 收到内部指令
    void ReceiveCommand(char* const& buf, size_t const& len) override;
};
