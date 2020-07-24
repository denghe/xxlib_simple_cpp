#pragma once
#include "peer.h"
namespace EP = xx::Epoll;

// 拨号到 lobby 服务 产生的 peer
struct LPeer : Peer {
    // 继承构造函数
    using Peer::Peer;

    // 收到正常包
    void ReceivePackage(char* const& buf, size_t const& len) override;

    // 收到首包
    void ReceiveFirstPackage(char* const& buf, size_t const& len) override;

    // todo
    bool Close(int const &reason, char const* const& desc) override;
};
