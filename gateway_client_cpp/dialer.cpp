#include "dialer.h"
#include "client.h"
#include "peer.h"

Client &Dialer::GetClient() {
    // 拿到服务上下文
    return *(Client *) ep;
}

EP::TcpPeer_u Dialer::OnCreatePeer() {
    // 返回 Peer 类实例
    return xx::TryMakeU<Peer>();
}

void Dialer::OnConnect(EP::TcpPeer_r const &peer_) {
    // 没连上
    if (!peer_) return;

    std::cout << "dialer connected." << std::endl;

    // 将 peer 放入容器
    GetClient().peer = peer_.As<Peer>();
}
