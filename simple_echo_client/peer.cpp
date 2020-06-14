#include "peer.h"
#include "header.h"

void Peer::OnDisconnect(int const &reason) {
    std::cout << "OnDisconnect. reason = " << reason << std::endl;
}

int Peer::SendPackage(char const *const &buf, size_t const &len) {
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

void Peer::OnReceive() {
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

void Peer::HandleReceive(char *buf, size_t len) {
    // 这里先输出收到的内容
    std::cout << "recv message from server: " << std::string_view(buf, len) << std::endl;

    // todo: 内容处理
}
