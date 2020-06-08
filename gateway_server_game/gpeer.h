#pragma once

#include "peer.h"

// gateway 连进来产生的 peer
struct GPeer : Peer {
    // 继承构造函数
    using Peer::Peer;

    // 从相应容器中移除
    ~GPeer();

    // 收到正常包
    void OnReceivePackage(char *const &buf, size_t const &len) override;

    // 收到首包( 拿到 gatewayId, 放入相应容器 )
    void OnReceiveFirstPackage(char *const &buf, size_t const &len) override;
};
