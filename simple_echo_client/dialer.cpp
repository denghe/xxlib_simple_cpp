#include "dialer.h"
#include "client.h"

void Dialer::OnConnect(std::shared_ptr<Peer> const &peer) {
    // 没连上
    if (!peer) return;

    // 加持
    peer->Hold();

    // 拿到 client 上下文
    auto &&c = xx::As<Client>(ec);

    // 将 peer 存放到 c 备用( 再次加持 )
    c->peer = peer;

    std::cout << "OnConnect" << std::endl;
}
