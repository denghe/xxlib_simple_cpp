#include "dialer.h"
#include "server.h"
#include "lpeer.h"
#include "config.h"

Server &Dialer::GetServer() {
    // 拿到服务上下文
    return *(Server *) ep;
}

EP::TcpPeer_u Dialer::OnCreatePeer() {
    // 返回 Peer 类实例
    return xx::TryMakeU<LPeer>();
}

void Dialer::OnConnect(EP::TcpPeer_r const &peer_) {
    // 没连上
    if (!peer_) return;

    // 转为具体类型
    auto &&sp = peer_.As<LPeer>();

    // 将 peer 放入容器
    GetServer().lobbyPeer = sp;

    // 向 lobby 发送自己的 serverId
    sp->SendCommand("serverId", config.serverId);
}
