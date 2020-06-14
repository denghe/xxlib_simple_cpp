#include "cpeer.h"
#include "speer.h"
#include "server.h"
#include "config.h"

bool CPeer::Close(int const& reason) {
    // close fd 解绑 并触发 OnDisconnect
    if (this->Peer::Close(reason)) {
        // 群发断开通知并解除 client id 映射, 从容器移除, 转为 Hold 持有
        DelayClose(0);
        // 延迟自杀
        DelayUnhold();
        return true;
    }
    return false;
}

void CPeer::DelayClose(double const& delaySeconds) {
    // 避免重复执行
    if (closed || !Alive()) return;
    // 标记为延迟自杀
    closed = true;
    // 兼容 Close 直接关闭
    if (delaySeconds > 0) {
        // 主动调用 OnDisconnect
        OnDisconnect(__LINE__);
        // 利用超时来 Close
        SetTimeoutSeconds(delaySeconds);
    }
    // 群发断开通知
    for (auto &&sid : serverIds) {
        GetServer().dps[sid].second->SendCommand("disconnect", clientId);
    }
    // 从容器移除( 减持 )
    GetServer().cps.erase(clientId);
}

void CPeer::OnReceivePackage(char *const &buf, size_t const &len) {
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
    GetServer().dps[sid].second->Send({buf - 4, len + 4});

    std::cout << "recv client package. serverId = " << sid << " len = " << len << std::endl;
}

void CPeer::OnReceiveCommand(char *const &buf, size_t const &len) {
    // 续命
    SetTimeoutSeconds(config.clientTimeoutSeconds);

    // echo 发回( buf 指向了 header + 0xffffffff 之后的区域，故 -8 指向 header )
    Send({buf - 8, len + 8});
}

void CPeer::OnDisconnect(int const &reason) {
    std::cout << "" << std::endl;
}
