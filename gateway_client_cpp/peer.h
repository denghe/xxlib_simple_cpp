#pragma once

#include "xx_epoll.h"
#include "xx_datareader.h"
#include <deque>

namespace EP = xx::Epoll;

// 预声明
struct Client;

// 继承 默认 连接覆盖收包函数
struct Peer : EP::TcpPeer {
    // 先用 一个简单结构 来存放收到的包. uint32: serverId( 谁发的 )
    std::deque<std::pair<uint32_t, xx::Data>> packages;

    // 已开放的 server id 列表( 发送时如果目标 id 不在列表里则忽略发送但不报错? )
    std::vector<uint32_t> openServerIds;

    // 拿到 client 上下文引用, 以方便写事件处理代码
    Client &GetClient();

    // 收到数据. 切割后进一步调用 OnReceivePackage 和 OnReceiveCommand
    void OnReceive() override;

    // 收到正常包
    void OnReceivePackage(uint32_t const &serverId, char *const &buf, size_t const &len);

    // 收到内部指令
    void OnReceiveCommand(char *const &buf, size_t const &len);

    // 构造数据包并发送. len(uint32) + serverId(uint32) + args...
    template<typename...Args>
    void SendTo(uint32_t const &serverId, Args const &... args) {
        xx::Data d;
        d.Reserve(1024);
        d.len = sizeof(uint32_t);
        d.WriteFixed(serverId);
        xx::Write(d, args...);
        *(uint32_t *) d.buf = (uint32_t) (d.len - sizeof(uint32_t));
        this->Send(std::move(d));
    }
};
