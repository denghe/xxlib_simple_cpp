#include "peer.h"
#include "client.h"

bool Peer::Close(int const& reason) {
    // 关闭 fd. 同时也是重入检测
    if (!this->Item::Close(reason)) return false;
    {
        // 从容器移除 this( 如果有放入的话 )
        ((Client *) &*ec)->peer.reset();
        std::cout << "peer disconnected. reason = " << reason << std::endl;
    }
    // 延迟减持
    DelayUnhold();
    return true;
}

bool Peer::IsOpened(uint32_t const& serverId) const {
    return std::find(openServerIds.begin(), openServerIds.end(), serverId) != openServerIds.end();
}

void Peer::Receive() {
    // 取出指针备用
    auto buf = recv.buf;
    auto end = recv.buf + recv.len;
    uint32_t dataLen = 0;
    uint32_t serverId = 0;

    // 确保包头长度充足
    while (buf + sizeof(dataLen) <= end) {
        // 取长度
        dataLen = *(uint32_t *) buf;

        // 长度异常则断线退出( 不含地址? 超长? 256k 不够可以改长 )
        if (dataLen < sizeof(serverId) || dataLen > 1024 * 256) {
            Close(__LINE__);
            return;
        }

        // 数据未接收完 就 跳出
        if (buf + sizeof(dataLen) + dataLen > end) break;

        // 跳到数据区开始调用处理回调
        buf += sizeof(dataLen);
        {
            // 取出地址( 手机上最好用 memcpy )
            serverId = *(uint32_t *) buf;

            // 包类型判断
            if (serverId == 0xFFFFFFFFu) {
                // 内部指令
                ReceiveCommand(buf + sizeof(serverId), dataLen - sizeof(serverId));
            } else {
                // 普通包. serverId 打头
                ReceivePackage(serverId, buf + sizeof(serverId), dataLen - sizeof(serverId));
            }

            // 如果当前类实例已自杀则退出
            if (!Alive()) return;
        }
        // 跳到下一个包的开头
        buf += dataLen;
    }

    // 移除掉已处理的数据( 将后面剩下的数据移动到头部 )
    recv.RemoveFront(buf - recv.buf);
}

void Peer::ReceivePackage(uint32_t const &serverId, char *const &buf, size_t const &len) {
    // 试读出序号. 出错直接断开退出
    int serial = 0;
    xx::DataReader dr(buf, len);
    if (dr.Read(serial)) {
        Close(__LINE__);
        return;
    }
    // 剩余数据打包为 xx::Data 塞到收包队列
    receivedPackages.emplace_back(serverId, serial, xx::Data(buf + dr.offset, len - dr.offset));
}

void Peer::ReceiveCommand(char *const &buf, size_t const &len) {
    // 要填充的变量
    std::string cmd;
    uint32_t serverId = 0;

    // 读取失败直接断开
    if (int r = xx::Read(buf, len, cmd, serverId)) {
        Close(__LINE__);
        return;
    }

    if (cmd == "open") {
        std::cout << "peer recv cmd: open " << serverId << std::endl;
        // serverId 放入白名单
        openServerIds.emplace_back(serverId);

    } else if (cmd == "close") {
        std::cout << "peer recv cmd: close " << serverId << std::endl;
        // serverId 从白名单移除
        openServerIds.erase(std::find(openServerIds.begin(), openServerIds.end(), serverId));
        if (openServerIds.empty()) {
            Close(__LINE__);
            return;
        }
    }
    else {
        std::cout << "peer recv cmd: unknown " << std::endl;
        Close(__LINE__);
        return;
    }
}
