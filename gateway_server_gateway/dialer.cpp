#include "dialer.h"
#include "server.h"
#include "speer.h"
#include "config.h"

Server &Dialer::GetServer() {
    // 拿到服务上下文
    return *(Server *) ep;
}

EP::TcpPeer_u Dialer::OnCreatePeer() {
    // 返回 Peer 类实例
    return xx::TryMakeU<SPeer>();
}

void Dialer::OnConnect(EP::TcpPeer_r const &peer_) {
    // 没连上
    if (!peer_) return;

    // 转为具体类型
    auto &&sp = peer_.As<SPeer>();

    // 将 peer 放入容器
    GetServer().dps[serverId].second = sp;

    // 继续填充一些依赖
    sp->serverId = serverId;

    // 向 server 发送自己的 gatewayId
    sp->SendCommand_GatewayId(config.gatewayId);
}
