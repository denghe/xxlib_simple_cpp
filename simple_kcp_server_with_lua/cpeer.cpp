#include "cpeer.h"
#include "server.h"
#include "xx_logger.h"

bool CPeer::Close(int const& reason, char const* const& desc) {
    // 防重入( 同时关闭 fd )
    if (!this->BaseType::Close(reason, desc)) return false;
    LOG_INFO("CPeer Close. ip = ", addr," reason = ", reason, ", desc = ", desc);
    // 从容器移除( 减持 )
    GetServer().cps.erase(this);
    // 延迟减持
    DelayUnhold();
    return true;
}

void CPeer::DelayClose(double const& delaySeconds) {
    // 这个的日志记录在调用者那里
    // 避免重复执行
    if (closed || !Alive()) return;
    // 标记为延迟自杀
    closed = true;
    // 利用超时来 Close
    SetTimeoutSeconds(delaySeconds <= 0 ? 3 : delaySeconds);
    // 从容器移除( 减持 )
    GetServer().cps.erase(this);
}

Server &CPeer::GetServer() const {
    // 拿到服务上下文
    return *(Server *) &*ec;
}

void CPeer::Receive() {
    // 如果属于延迟踢人拒收数据状态，直接清数据短路退出
    if (closed) {
        recv.Clear();
        return;
    }

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
            Close(-__LINE__, " KPeer Receive if (dataLen < sizeof(addr) || dataLen > 1024 * 256)");
            return;
        }

        // 数据未接收完 就 跳出
        if (buf + sizeof(dataLen) + dataLen > end) break;

        // 跳到数据区开始调用处理回调
        buf += sizeof(dataLen);
        {
            // 调用包处理函数
            ReceivePackage(buf, dataLen);

            // 如果当前类实例 fd 已 close 则退出
            if (!Alive() || closed) return;
        }
        // 跳到下一个包的开头
        buf += dataLen;
    }

    // 移除掉已处理的数据( 将后面剩下的数据移动到头部 )
    recv.RemoveFront(buf - recv.buf);
}

void CPeer::ReceivePackage(char *const &buf, size_t const &len) {
    LOG_INFO("CPeer ReceivePackage. ip = ", addr, ", buf len = ", len);
    // todo: logic here
}
