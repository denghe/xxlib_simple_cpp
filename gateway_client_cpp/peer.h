#pragma once

#include "xx_epoll.h"
#include "xx_datareader.h"
#include <deque>

namespace EP = xx::Epoll;

// 预声明
struct Client;

// 继承 默认 连接覆盖收包函数
struct Peer : EP::TcpPeer {
    // 先用 一个简单结构 来模拟收到的包
    std::deque<std::pair<std::string, uint32_t>> packages;

    // 拿到 client 上下文引用, 以方便写事件处理代码
    Client &GetClient();

    // 收到数据. 切割后进一步调用 OnReceivePackage 和 OnReceiveCommand
    void OnReceive() override;

    // 收到正常包
    void OnReceivePackage(char *const &buf, size_t const &len);

    // 收到内部指令
    void OnReceiveCommand(char *const &buf, size_t const &len);

    // 开始向 data 写包. 空出 长度 头部
    static void WritePackageBegin(xx::Data &d, size_t const &reserveLen, uint32_t const &addr);

    // 结束写包。根据数据长度 填写 包头
    static void WritePackageEnd(xx::Data &d);

    // 构造内部指令包. LEN + ADDR + cmd string + args...
    template<typename...Args>
    void SendCommand(Args const &... cmdAndArgs) {
        xx::Data d;
        WritePackageBegin(d, 1024, 0xFFFFFFFFu);
        xx::Write(d, cmdAndArgs...);
        WritePackageEnd(d);
        this->Send(std::move(d));
    }
};
