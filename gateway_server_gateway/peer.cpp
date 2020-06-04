#include "peer.h"
#include "server.h"
#include "xx_datareader.h"

Server &Peer::GetServer() {
    // 拿到服务上下文
    return *(Server *) ep;
}

void Peer::OnReceive() {
    // 如果属于延迟踢人拒收数据状态，直接清数据短路退出
    if (!allowReceive) {
        recv.Clear();
        return;
    }

    // 死亡判断变量
    EP::Ref<Item> alive(this);

    // 取出指针备用
    auto buf = recv.buf;
    auto end = recv.buf + recv.len;
    uint32_t dataLen = 0;

    // 确保包头长度充足
    while (buf + sizeof(dataLen) <= end) {
        // 取长度
        dataLen = *(uint32_t *) buf;

        // 长度异常则断线退出
        if (dataLen < 8 || dataLen > 1024 * 256) {
            Dispose();
            return;
        }

        // 数据未接收完 就 跳出
        if (buf + sizeof(dataLen) + dataLen > end) break;

        // 跳到数据区开始调用处理回调
        buf += 4;
        {
            // 包类型判断. 地址为 特殊值 则为 内部指令
            if (*(uint32_t *) buf == 0xFFFFFFFFu) {
                // 传参时跳过 id 部分
                OnReceiveCommand(buf + sizeof(dataLen), dataLen - sizeof(dataLen));
            } else {
                // id 打头。 -4 可定位到 长度包头开始的地址
                OnReceivePackage(buf, dataLen);
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

// 开始向 data 写包. 空出 长度 头部
void Peer::WritePackageBegin(xx::Data &d, size_t const &reserveLen, uint32_t const &addr) {
    d.Reserve(4 + reserveLen);
    d.len = 4;
    d.WriteFixed(addr);
}

// 结束写包。根据数据长度 填写 包头
void Peer::WritePackageEnd(xx::Data &d) {
    *(uint32_t *) d.buf = (uint32_t) (d.len - 4);
}

// 构造内部指令包. cmd string + args...
template<typename...Args>
void Peer::SendCommand(Args const &... cmdAndArgs) {
    xx::Data d;
    WritePackageBegin(d, 1024, 0xFFFFFFFFu);
    xx::Write(d, cmdAndArgs...);
    WritePackageEnd(d);
    this->Send(std::move(d));
}
