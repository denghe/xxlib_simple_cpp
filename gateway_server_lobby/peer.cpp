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
            Close(__LINE__);
            return;
        }

        // 数据未接收完 就 跳出
        if (buf + sizeof(dataLen) + dataLen > end) break;

        // 跳到数据区开始调用处理回调
        buf += sizeof(dataLen);
        {
            if (LIKELY(id)) {
                // 处理 ping 指令. 直接将内容 echo 回去
                if (dataLen > sizeof(dataLen) && *(uint32_t*)(buf + dataLen) == 0xFFFFFFFFu) {
                    Send(buf + dataLen - sizeof(dataLen), dataLen + sizeof(dataLen));
                }
                else {
                    // 调用处理函数
                    ReceivePackage(buf, dataLen);
                }
            } else {
                // 调用处理首包函数
                ReceiveFirstPackage(buf, dataLen);
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
