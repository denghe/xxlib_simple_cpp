#include "server.h"
#include "peer.h"
#include "phandler.h"
#include "aphandler.h"
#include "gphandler.h"

void Peer::SetAPHandler() {
    assert(!phandler);
    // 创建处理类并关联, 处理类创建过程中会将 peer 放入相应容器
    phandler = xx::MakeU<APHandler>(*this, ++GetServer().autoIncId);
}

void Peer::SetGPHandler(uint32_t const &gatewayId) {
    assert(phandler);
    // 创建处理类并关联( 会导致之前的 peer handler 析构 并从相应容器移除 ) 处理类创建过程中会将 peer 放入相应容器
    phandler = xx::MakeU<GPHandler>(*this, gatewayId);
}

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
    uint32_t addr = 0;

    // 确保包头长度充足
    while (buf + sizeof(dataLen) <= end) {
        // 取长度
        dataLen = *(uint32_t *) buf;

        // 长度异常则断线退出( 不含地址? 超长? 256k 不够可以改长 )
        if (dataLen < sizeof(addr) || dataLen > 1024 * 256) {
            Dispose();
            return;
        }

        // 数据未接收完 就 跳出
        if (buf + sizeof(dataLen) + dataLen > end) break;

        // 跳到数据区开始调用处理回调
        buf += sizeof(dataLen);
        {
            // 取出地址
            addr = *(uint32_t *) buf;

            // 包类型判断
            if (addr == 0xFFFFFFFFu) {
                // 内部指令. 传参时跳过 addr 部分
                OnReceiveCommand(buf + sizeof(addr), dataLen - sizeof(addr));
            } else {
                // 普通包. id 打头
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


void Peer::OnReceivePackage(char *const &buf, size_t const &len) {
    phandler->OnReceivePackage(buf, len);
}

void Peer::OnReceiveCommand(char *const &buf, size_t const &len) {
    phandler->OnReceiveCommand(buf, len);
}

void Peer::OnDisconnect(int const &reason) {
    phandler->OnDisconnect(reason);
}


// 开始向 data 写包. 跳过 长度 头部不写, 写入地址
void Peer::WritePackageBegin(xx::Data &d, size_t const &reserveLen, uint32_t const &addr) {
    d.Reserve(4 + reserveLen);
    d.len = 4;
    d.WriteFixed(addr);
}

// 结束写包。根据数据长度 填写 包头
void Peer::WritePackageEnd(xx::Data &d) {
    *(uint32_t *) d.buf = (uint32_t) (d.len - 4);
}
