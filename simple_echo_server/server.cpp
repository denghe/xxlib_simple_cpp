#include "server.h"

void Listener::OnAccept(std::shared_ptr<Peer> const &peer) {
    // 放入 ec->holdItems 以确保 智能指针不自杀
    peer->Retain();

    // 设置 n 秒后断线( 触发 OnDisconnect )
    peer->SetTimeoutSeconds(30);

    std::cout << "client(" << EP::AddressToString(peer->addr) << ") connected." << std::endl;
}


void Peer::OnDisconnect(int const &reason) {
    // 从 ec->holdItems 延迟移除 以 释放智能指针( 出函数后 )
    AutoRelease();

    std::cout << EP::AddressToString(addr) << " disconnected. reason = " << reason << std::endl;
}

int Peer::SendPackage(char const *const &buf, size_t const &len) {
    // 最大长度检测
    if (len > std::numeric_limits<decltype(Header::len)>::max()) return -9999;

    // 构造包头
    Header h{};
    h.len = (decltype(Header::len)) len;

    // 构造包数据容器并拼接
    xx::Data d(sizeof(h) + len);
    d.WriteBuf((char *) &h, sizeof(h));
    d.WriteBuf(buf, len);

    // 发送
    return Send(std::move(d));
}

void Peer::OnReceive() {
    // 包头容器
    Header h{};

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

        // 如果当前类实例已 Close 则退出
        if (fd == -1) return;

        // 跳到下一个包的开头
        offset += h.len;
    }

    // 移除掉已处理的数据( 将后面剩下的数据移动到头部 )
    recv.RemoveFront(offset);
}

void Peer::HandleReceive(char *buf, size_t len) {
    // 这里先输出收到的内容
    std::cout << "recv message from client(" << EP::AddressToString(addr) << "): " << std::string_view(buf, len)
              << std::endl;

    // todo: 内容处理

    // 先实现 echo
    SendPackage(buf, len);

    // 如果内容合法, 则重新设置 n 秒后断线( keep alive 的效果 )
    SetTimeoutSeconds(30);
}

int Server::FrameUpdate() {
    return 0;
}

std::string Server::Init() {
    // 按配置的端口创建监听器
    xx::MakeTo(listener, shared_from_this());
    if(listener->Listen(config.listenPort)) {
        return std::string("listen to port ") + std::to_string(config.listenPort) + " failed.";
    }
    return "";
}

void Server::Dispose() {
    // 清理监听器( 消除对 Context 的引用计数的影响 )
    listener.reset();
}
