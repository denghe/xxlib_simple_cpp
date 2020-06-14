#pragma once
#include "peer.h"
namespace EP = xx::Epoll;

// 拨号到 lobby 服务 产生的 peer
struct LPeer : Peer {
    // 继承构造函数
    using Peer::Peer;

    // 收到正常包
    void OnReceivePackage(char* const& buf, size_t const& len) override;

    // 收到首包
    void OnReceiveFirstPackage(char* const& buf, size_t const& len) override;

    // 断开时 清除所有 client peer 中的 相关 open id. 列表被清空则踢掉
    void OnDisconnect(int const &reason) override;
};
