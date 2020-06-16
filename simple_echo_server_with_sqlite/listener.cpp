#include "listener.h"
#include "peer.h"

void Listener::Accept(std::shared_ptr<Peer> const &peer) {
    // 放入 ec->holdItems 以确保 智能指针不自杀
    peer->Hold();

    // 设置 n 秒后断线( 触发 OnDisconnect )
    peer->SetTimeoutSeconds(30);

    std::cout << "client(" << EP::AddressToString(peer->addr) << ") connected." << std::endl;
}
