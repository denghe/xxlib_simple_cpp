#include "server.h"
#include "peer.h"

Server &Peer::GetServer() {
    // 拿到服务上下文
    return *(Server *) &*ec;
}

void Peer::Receive() {
    // 取出指针备用
    auto buf = recv.buf;
    auto end = recv.buf + recv.len;
    uint32_t dataLen = 0;

    // 确保包头长度充足
    while (buf + sizeof(dataLen) <= end) {
        // 取长度
        dataLen = *(uint32_t *) buf;

        // 长度异常则断线退出( 超长? 256k 不够可以改长 )
        if (UNLIKELY(dataLen > 1024 * 256)) {
            Close(__LINE__, __LINESTR__ " Peer Receive dataLen > 1024 * 256");
            return;
        }

        // 数据未接收完 就 跳出
        if (buf + sizeof(dataLen) + dataLen > end) break;

        // 跳到数据区开始调用处理回调
        buf += sizeof(dataLen);
        {
            if (LIKELY(id)) {
                ReceivePackage(buf, dataLen);
            } else {
                ReceiveFirstPackage(buf, dataLen);
            }
            // 如果当前类实例已close则退出
            if (!Alive()) return;
        }
        // 跳到下一个包的开头
        buf += dataLen;
    }

    // 移除掉已处理的数据( 将后面剩下的数据移动到头部 )
    recv.RemoveFront(buf - recv.buf);
}
