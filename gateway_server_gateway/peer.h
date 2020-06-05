#pragma once
#include "xx_epoll.h"
#include "xx_datareader.h"
#include <unordered_set>
namespace EP = xx::Epoll;

// 预声明
struct Server;

// 继承 默认 连接覆盖收包函数
struct Peer : EP::TcpPeer {
    // 是否允许处理收到的数据( 用于实现延迟掐线 ), 在 OnReceive 入口判断. 不允许就直接抛弃数据
    bool allowReceive = true;

    // 拿到 server 上下文引用, 以方便写事件处理代码
    Server &GetServer();

    // 收到数据. 切割后进一步调用 OnReceivePackage 和 OnReceiveCommand
    void OnReceive() override;

    // 收到正常包
    virtual void OnReceivePackage(char* const& buf, std::size_t const& len) = 0;

    // 收到内部指令
    virtual void OnReceiveCommand(char* const& buf, std::size_t const& len) = 0;

    // 开始向 data 写包. 空出 长度 头部
    static void WritePackageBegin(xx::Data& d, size_t const& reserveLen, uint32_t const& addr);

    // 结束写包。根据数据长度 填写 包头
    static void WritePackageEnd(xx::Data& d);

    // 构造内部指令包. cmd string + args...
    template<typename...Args>
    void SendCommand(Args const &... cmdAndArgs) {
        xx::Data d;
        WritePackageBegin(d, 1024, 0xFFFFFFFFu);
        xx::Write(d, cmdAndArgs...);
        WritePackageEnd(d);
        this->Send(std::move(d));
    }
};
