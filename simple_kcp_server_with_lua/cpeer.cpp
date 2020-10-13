#include "cpeer.h"
#include "server.h"
#include "xx_logger.h"

bool CPeer::Close(int const& reason, char const* const& desc) {
    // 调用基类关闭函数 并确保执行成功后继续
    if (!this->BaseType::Close(reason, desc)) return false;

    LOG_INFO("ip = ", addr," reason = ", reason, ", desc = ", desc);

    // 延迟减持( 与 Listener::Accept 的 Hold 对应 )
    DelayUnhold();

    // 返回执行成功
    return true;
}

void CPeer::Receive() {
    // 取出指针备用
    auto buf = recv.buf;
    auto end = recv.buf + recv.len;
    uint32_t dataLen = 0;

    // 确保包头长度充足
    while (buf + sizeof(dataLen) <= end) {
        // 取长度
        dataLen = *(uint32_t *) buf;

        // 长度异常则断线退出( 不含地址? 超长? 256k 不够可以改长 )
        if (dataLen > 1024 * 256) {
            Close(-__LINE__, " CPeer Receive if (dataLen < sizeof(addr) || dataLen > 1024 * 256)");
            return;
        }

        // 数据未接收完 就 跳出
        if (buf + sizeof(dataLen) + dataLen > end) break;

        // 跳到数据区开始调用处理回调
        buf += sizeof(dataLen);
        {
            // 调用包处理函数
            ReceivePackage(buf, dataLen);

            // 如果当前类实例已 close 则退出
            if (!Alive()) return;
        }
        // 跳到下一个包的开头
        buf += dataLen;
    }

    // 移除掉已处理的数据( 将后面剩下的数据移动到头部 )
    recv.RemoveFront(buf - recv.buf);
}

void CPeer::ReceivePackage(char *const &buf, size_t const &len) {
    LOG_INFO("ip = ", addr, ", buf len = ", len);

    // todo: logic here

    // 如果收到合法数据：续命
    SetTimeoutSeconds(5);

    // 先实现原样发回的逻辑 ( 地址 - 4 才能包含长度包头 )
    Send(buf - 4, len + 4);
    Flush();
}
