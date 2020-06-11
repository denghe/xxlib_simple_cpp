#pragma once

#include "peer.h"

// server 连进来产生的 peer
struct SPeer : Peer {
    // 继承构造函数
    using Peer::Peer;

    // 从相应容器中移除
    ~SPeer() override;

    // 收到正常包
    void OnReceivePackage(char *const &buf, size_t const &len) override;

    // 收到首包( 拿到 serverId, 放入相应容器 )
    void OnReceiveFirstPackage(char *const &buf, size_t const &len) override;
};
