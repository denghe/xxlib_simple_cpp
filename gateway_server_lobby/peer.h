#pragma once

#include "xx_epoll.h"
#include "xx_datareader.h"
#include <unordered_set>

namespace EP = xx::Epoll;

// 预声明
struct Server;

// 所有 peer 的基类. 所有数据包共有结构: 以 4 字节 数据长度 打头
struct Peer : EP::TcpPeer {
    // 服务编号( 同时用作首包标志位. 首包之后将非 0 )
    uint32_t id = 0;

    // 继承构造函数
    using EP::TcpPeer::TcpPeer;

    // 拿到 server 上下文引用, 以方便写事件处理代码
    Server &GetServer();

    // 收到数据并切割. 切割后进一步调用 ReceiveFirstPackage 和 ReceivePackage
    void Receive() override;

    // 收到包
    virtual void ReceivePackage(char *const &buf, size_t const &len) = 0;

    // 收到首包
    virtual void ReceiveFirstPackage(char *const &buf, size_t const &len) = 0;
};
