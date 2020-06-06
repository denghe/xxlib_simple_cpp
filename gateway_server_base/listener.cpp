#include "listener.h"
#include "server.h"
#include "peer.h"
#include "config.h"

EP::TcpPeer_u Listener::OnCreatePeer() {
    // 返回 Peer 类实例
    return xx::TryMakeU<Peer>();
}

void Listener::OnAccept(EP::TcpPeer_r const &peer_) {
    // 没连上
    if (!peer_) return;

    // 转为本来的类型
    auto&& p = peer_.As<Peer>();

    // 设置使用 匿名处理类
    p->SetAPHandler();
}
