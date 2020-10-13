#pragma once
#include "xx_epoll_kcp.h"
namespace EP = xx::Epoll;

// 预声明
struct Server;

// 客户端 连进来产生的 peer
struct CPeer : EP::KcpPeer {
	using BaseType = EP::KcpPeer;
    using BaseType::BaseType;

    // DelayUnhold 自杀( 与 Listener::Accept 的 Hold 对应 )
    bool Close(int const& reason, char const* const& desc) override;

    // 收到数据. 切割后进一步调用 ReceivePackage
    void Receive() override;

    // 收到包
    void ReceivePackage(char* const& buf, size_t const& len);
};
