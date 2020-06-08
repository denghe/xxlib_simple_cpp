#pragma once

#include "xx_epoll.h"
#include "xx_datareader.h"
#include "phandler.h"
#include <unordered_set>

namespace EP = xx::Epoll;

// 预声明
struct Server;

// 被 gateway, game servers 连 产生的 peer
struct SPeer : EP::TcpPeer {
    // 可后期绑定的事件处理类
    std::unique_ptr<PHandler<SPeer>> phandler;

    // 将事件处理类设置为具体派生类型
    void SetAPHandler();
    void SetSPHandler(uint32_t const& serverId);

    // 拿到 server 上下文引用, 以方便写事件处理代码
    Server &GetServer();

    // 收到数据. 切割后进一步调用 OnReceivePackage 和 OnReceiveCommand
    void OnReceive() override;

    // 收到正常包
    virtual void OnReceivePackage(char *const &buf, size_t const &len);

    // 收到内部指令
    virtual void OnReceiveCommand(char *const &buf, size_t const &len);

    // 断开事件
    void OnDisconnect(int const &reason) override;

    // 下面是一些拼包辅助函数

    // 开始向 data 写包. 空出 长度 头部
    static void WritePackageBegin(xx::Data &d, size_t const &reserveLen, uint32_t const &addr);

    // 结束写包。根据数据长度 填写 包头
    static void WritePackageEnd(xx::Data &d);

    // 构造内部指令包. cmd string + args...
    template<typename...Args>
    void SendCommand(Args const &... cmdAndArgs) {
        xx::Data d;
        WritePackageBegin(d, 1024, 0xFFFFFFFFu);
        xx::Write(d, cmdAndArgs...);
        WritePackageEnd(d);
        Send(std::move(d));
    }
};
