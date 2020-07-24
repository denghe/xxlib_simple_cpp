#pragma once

#include "peer.h"

// gateway 连进来产生的 peer
struct GPeer : Peer {
    // 继承构造函数
    using Peer::Peer;

    // 从相应容器中移除
    bool Close(int const &reason, char const* const& desc) override;

    // 收到正常包
    void ReceivePackage(char *const &buf, size_t const &len) override;

    // 收到首包( 拿到 gatewayId, 放入相应容器 )
    void ReceiveFirstPackage(char *const &buf, size_t const &len) override;
};
