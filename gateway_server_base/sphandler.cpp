#include "sphandler.h"
#include "peer.h"
#include "server.h"

SPHandler::SPHandler(Peer& peer, uint32_t const& id) : PHandler(peer, id) {
    // 放入容器
    peer.GetServer().sps[id] = &peer;
}

SPHandler::~SPHandler() {
    // 从容器移除
    peer.GetServer().sps.erase(id);
}

void SPHandler::OnReceivePackage(char *const &buf, size_t const &len) {
    // todo: 正常通信
}

void SPHandler::OnReceiveCommand(char *const &buf, size_t const &len) {
    // 触发断线事件并自杀( 服务之间平时不走 cmd 指令 )
    peer.OnDisconnect(__LINE__);
    peer.Dispose();
}
