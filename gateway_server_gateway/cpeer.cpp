#include "cpeer.h"
#include "speer.h"
#include "server.h"
#include "config.h"

bool CPeer::Close(int const& reason) {
    // 防重入( 同时关闭 fd )
    if (!this->BaseType::Close(reason)) return false;
    // 群发断开指令 并从容器移除
    PartialClose();
    // 延迟减持
    DelayUnhold();
    return true;
}

void CPeer::DelayClose(double const& delaySeconds) {
    // 避免重复执行
    if (closed || !Alive()) return;
    // 标记为延迟自杀
    closed = true;
    // 利用超时来 Close
    SetTimeoutSeconds(delaySeconds <= 0 ? 3 : delaySeconds);
    // 群发断开指令 并从容器移除
    PartialClose();
}

void CPeer::PartialClose() {
    // 群发断开通知
    for (auto &&sid : serverIds) {
        GetServer().dps[sid].second->SendCommand("disconnect", clientId);
    }
    // 从容器移除( 减持 )
    GetServer().cps.erase(clientId);
}

void CPeer::ReceivePackage(char *const &buf, size_t const &len) {
    // 取出 serverId
    auto sid = *(uint32_t *) buf;

    // 判断该服务编号是否在白名单中. 找不到就忽略
    if (serverIds.find(sid) == serverIds.end()) {
        std::cout << "recv client package. can't find serverId = " << sid << ". serverIds = [" << std::endl;
        for(auto&& id : serverIds) {
            std::cout << " " << id;
        }
        std::cout << " ]" << std::endl;
        return;
    }

    // 续命. 每次收到合法数据续一下
    SetTimeoutSeconds(config.clientTimeoutSeconds);

    // 篡改 serverId 为 clientId
    *(uint32_t *) buf = clientId;

    // 用 serverId 对应的 peer 转发完整数据包
    GetServer().dps[sid].second->Send(buf - 4, len + 4);

    std::cout << "recv client package. serverId = " << sid << " len = " << len << std::endl;
}

void CPeer::ReceiveCommand(char *const &buf, size_t const &len) {
    // 续命
    SetTimeoutSeconds(config.clientTimeoutSeconds);

    // echo 发回( buf 指向了 header + 0xffffffff 之后的区域，故 -8 指向 header )
    Send(buf - 8, len + 8);
}
