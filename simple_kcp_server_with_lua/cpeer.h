#pragma once
#include "xx_epoll_kcp.h"
#include "xx_data_rw.h"
#include <unordered_set>
namespace EP = xx::Epoll;

// 预声明
struct Server;

// 客户端 连进来产生的 peer
struct CPeer : EP::KcpPeer {
	using BaseType = EP::KcpPeer;
    using BaseType::BaseType;

    // 是否已关闭. true: 拒收数据, 且断线时不再次执行 Dispose  ( 主用于 延迟掐线 )
    bool closed = false;

    // 自增编号, accept 时填充
    uint32_t clientId = 0xFFFFFFFFu;

    // 拿到 server 上下文引用, 以方便写事件处理代码
    Server &GetServer() const;

    // 群发断开指令, 从容器移除变野,  DelayUnhold 自杀
    bool Close(int const& reason, char const* const& desc) override;

    // 延迟关闭( 设置 closed = true, 群发断开指令, 从容器移除变野, 靠超时自杀 )
    void DelayClose(double const& delaySeconds);

    // 收到数据. 切割后进一步调用 ReceivePackage
    void Receive() override;

    // 收到正常包
    void ReceivePackage(char* const& buf, size_t const& len);
};
