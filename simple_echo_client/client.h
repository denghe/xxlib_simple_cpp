#pragma once

#include "xx_epoll_sp.h"

namespace EP = xx::Epoll;

// 预声明
struct Dialer;
struct Peer;

// 服务本体
struct Client : EP::Context {
    using EP::Context::Context;

    // 进入时初始化下面的类成员( 构造函数里做不了, 因为无法 shared_from_this ). 退出时清理这些类成员( 去除引用计数 )
    int Run(double const &frameRate) override;

    // 在帧逻辑里自动拨号到服务器
    int FrameUpdate() override;

    // 拨号器
    std::shared_ptr<Dialer> dialer;

    // 拨号器连接成功后将 peer 存在此以便使用
    std::shared_ptr<Peer> peer;
};


// 继承默认监听器覆盖关键函数
struct Dialer : EP::TcpDialer<Peer> {
    using EP::TcpDialer<Peer>::TcpDialer;

    // 连接已建立, 搞事
    void OnConnect(std::shared_ptr<Peer> const &peer) override;
};


// 继承 默认 连接覆盖收包函数
struct Peer : EP::TcpPeer {
    using EP::TcpPeer::TcpPeer;

    // 处理接收事件
    void HandleReceive(char *buf, size_t len);

    // 收到数据
    void OnReceive() override;

    // 断线事件
    void OnDisconnect(int const &reason) override;

    // 在数据前面附带上 长度 并发送. 返回 非0 表示出状况( 但不一定是断线 )
    int SendPackage(char const *const &buf, size_t const &len);
};


// 包头. 包结构简单的定为  header(len) + data
struct Header {
    uint16_t len;
};


inline void Dialer::OnConnect(std::shared_ptr<Peer> const &peer) {
    // 没连上
    if (!peer) return;

    // 拿到 client 上下文
    auto &&c = xx::As<Client>(ec);

    // 将 peer 存放到 c 备用
    c->peer = peer;

    std::cout << "OnConnect" << std::endl;
}

inline void Peer::OnDisconnect(int const &reason) {
    std::cout << "OnDisconnect. reason = " << reason << std::endl;
}

inline int Peer::SendPackage(char const *const &buf, size_t const &len) {
    // 最大长度检测
    if (len > std::numeric_limits<decltype(Header::len)>::max()) return -9999;

    // 构造包头
    Header h;
    h.len = (decltype(Header::len)) len;

    // 构造包数据容器并拼接
    xx::Data d(sizeof(h) + len);
    d.WriteBuf((char *) &h, sizeof(h));
    d.WriteBuf(buf, len);

    // 发送
    return Send(std::move(d));
}


inline void Peer::OnReceive() {
    // 包头容器
    Header h;

    // 数据偏移
    size_t offset = 0;

    // 确保包头长度充足
    while (offset + sizeof(h) <= recv.len) {

        // 拷贝数据头出来
        memcpy(&h, recv.buf + offset, sizeof(h));

        // 数据未接收完 就 跳出
        if (offset + sizeof(h) + h.len > recv.len) break;

        // 偏移量 指向 数据区
        offset += sizeof(h);

        // 调用处理函数
        HandleReceive(recv.buf + offset, h.len);

        // 如果当前 fd 已关闭 则退出
        if (!Alive()) return;

        // 跳到下一个包的开头
        offset += h.len;
    }

    // 移除掉已处理的数据( 将后面剩下的数据移动到头部 )
    recv.RemoveFront(offset);
}

inline void Peer::HandleReceive(char *buf, size_t len) {
    // 这里先输出收到的内容
    std::cout << "recv message from server: " << std::string_view(buf, len) << std::endl;

    // todo: 内容处理
}

inline int Client::Run(double const &frameRate) {
    // 初始化拨号器
    xx::MakeTo(dialer, shared_from_this());
    // 添加默认拨号地址
    dialer->AddAddress("127.0.0.1", 10000);
    // 注册交互指令
    EnableCommandLine();
    xx::ScopeGuard sgCmdLine([&] {
        // 反注册交互指令( 消除对 Context 的引用计数的影响 )
        DisableCommandLine();
        // 清理成员变量( 消除对 Context 的引用计数的影响 )
        dialer.reset();
        peer.reset();
    });

    cmds["send"] = [this](std::vector<std::string> const &args) {
        if (args.size() < 2) {
            std::cout << "send: miss data" << std::endl;
        } else if (args[1].size() > std::numeric_limits<decltype(Header::len)>::max()) {
            std::cout << "send: data's size > max len" << std::endl;
        } else if (!peer) {
            std::cout << "send: no peer" << std::endl;
        } else {
            peer->SendPackage(args[1].data(), args[1].size());
            std::cout << "send: success" << std::endl;
        }
    };

    cmds["quit"] = [this](auto args) {
        running = false;
    };

    cmds["exit"] = cmds["quit"];

    return this->EP::Context::Run(frameRate);
}

inline int Client::FrameUpdate() {
    // 自动拨号 & 重连逻辑
    // 如果未创建连接并且拨号器没有正在拨号, 就开始拨号
    if (!peer && !dialer->Busy()) {
        // 超时时间 2 秒
        dialer->DialSeconds(2);
    }
    return 0;
}
