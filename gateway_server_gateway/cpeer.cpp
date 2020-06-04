#include "cpeer.h"
#include "speer.h"
#include "server.h"

void CPeer::SendCommand_Open(uint32_t const& serverId) {
    SendCommand("open", serverId);
}

void CPeer::SendCommand_Close(uint32_t const& serverId) {
    SendCommand("close", serverId);
}


void CPeer::OnReceivePackage(char *const &buf, size_t const &len) {

}


void CPeer::OnReceiveCommand(char *const &buf, size_t const &len) {

}

void CPeer::OnDisconnect(int const &reason) {
    // 延迟掐线模式 已经触发过该事件, 故 跳过
    if(!allowReceive) return;



}

SPeer *CPeer::TryGetSPeer(uint32_t const &serverId) {
    auto &&dps = GetServer().dps;
    auto &&iter = dps.find(serverId);
    if (iter == dps.end()) return nullptr;
    return iter->second.second.Lock();
}
