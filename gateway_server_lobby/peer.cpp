﻿#include "server.h"
#include "peer.h"

Server &Peer::GetServer() {
    // 拿到服务上下文
    return *(Server *) ep;
}

void Peer::OnReceive() {
    // Disposed 判断变量
    EP::Ref<Item> alive(this);

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
            Dispose();
            return;
        }

        // 数据未接收完 就 跳出
        if (buf + sizeof(dataLen) + dataLen > end) break;

        // 跳到数据区开始调用处理回调
        buf += sizeof(dataLen);
        {
            if (LIKELY(id)) {
                OnReceivePackage(buf, dataLen);
            } else {
                OnReceiveFirstPackage(buf, dataLen);
            }
            // 如果当前类实例已自杀则退出
            if (!alive) return;
        }
        // 跳到下一个包的开头
        buf += dataLen;
    }

    // 移除掉已处理的数据( 将后面剩下的数据移动到头部 )
    recv.RemoveFront(buf - recv.buf);
}
