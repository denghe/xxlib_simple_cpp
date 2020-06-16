#include "dialer.h"
#include "client.h"
#include "peer.h"

Client &Dialer::GetClient() {
    // 拿到服务上下文
    return *(Client *) &*ec;
}

void Dialer::Connect(std::shared_ptr<Peer> const &peer) {
    // 没连上
    if (!peer) return;

    // 加持
    peer->Hold();

    // 将 peer 放入容器
    GetClient().peer = peer;

    std::cout << "dialer connected." << std::endl;
}
