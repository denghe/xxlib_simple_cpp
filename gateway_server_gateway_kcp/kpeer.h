#pragma once
#include "xx_epoll_kcp.h"
#include "xx_data_rw.h"
#include <unordered_set>
namespace EP = xx::Epoll;

// 预声明
struct Server;

// 继承 默认 连接覆盖收包函数
struct KPeer : EP::KcpPeer {
    // 是否已关闭. true: 拒收数据, 且断线时不再次执行 Dispose  ( 主用于 延迟掐线 )
    bool closed = false;

    // 继承构造函数
    using EP::KcpPeer::KcpPeer;

    // 拿到 server 上下文引用, 以方便写事件处理代码
    Server &GetServer() const;

    // 收到数据. 切割后进一步调用 ReceivePackage 和 ReceiveCommand
    void Receive() override;

    // 收到正常包
    virtual void ReceivePackage(char* const& buf, size_t const& len) = 0;

    // 收到内部指令
    virtual void ReceiveCommand(char* const& buf, size_t const& len) = 0;

    // 构造内部指令包. LEN + ADDR + cmd string + args...
    template<typename...Args>
    void SendCommand(Args const &... cmdAndArgs) {
        auto&& d = ec->data;
        d.Reserve(32);
        d.len = sizeof(uint32_t);
        d.WriteFixed(0xFFFFFFFFu);
        xx::DataWriter dw(d);
        dw.Write(cmdAndArgs...);
        *(uint32_t *) d.buf = (uint32_t) (d.len - sizeof(uint32_t));
        this->Send(d.buf, d.len);
    }
};
