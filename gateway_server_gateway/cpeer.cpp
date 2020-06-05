#include "cpeer.h"
#include "speer.h"
#include "server.h"
#include "config.h"

void CPeer::SendCommand_Open(uint32_t const &serverId) {
    SendCommand("open", serverId);
}

void CPeer::SendCommand_Close(uint32_t const &serverId) {
    SendCommand("close", serverId);
}

void CPeer::OnReceivePackage(char *const &buf, size_t const &len) {
    // 取出 serverId
    auto sid = *(uint32_t*) buf;

    // 判断该服务编号是否在白名单中. 找不到就忽略
    if (serverIds.find(sid) == serverIds.end()) return;

    // 续命. 每次收到合法数据续一下
    SetTimeoutSeconds(config.clientTimeoutSeconds);

    // 篡改 serverId 为 clientId
    *(uint32_t *)buf = clientId;

    // 用 serverId 对应的 peer 转发完整数据包
    GetServer().dps[sid].second->Send(buf - 4, len + 4);
}


void CPeer::OnReceiveCommand(char *const &buf, size_t const &len) {
    // 续命
    SetTimeoutSeconds(config.clientTimeoutSeconds);

    // echo 发回( buf 指向了 header + 0xffffffff 之后的区域，故 -8 指向 header )
    Send((char *) buf - 8, len + 8);
}

void CPeer::OnDisconnect(int const &reason) {
    // 延迟掐线模式 已经触发过该事件, 故 跳过
    if (!allowReceive) return;

    // 从字典移除
    GetServer().cps.erase(clientId);

    // 群发断开通知
    for (auto &&sid : serverIds) {
        GetServer().dps[sid].second->SendCommand_Disconnect(clientId);
    }
}

SPeer *CPeer::TryGetSPeer(uint32_t const &serverId) {
    auto &&dps = GetServer().dps;
    auto &&iter = dps.find(serverId);
    if (iter == dps.end()) return nullptr;
    return iter->second.second.Lock();
}
