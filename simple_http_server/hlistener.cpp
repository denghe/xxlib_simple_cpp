#include "hlistener.h"
#include "hpeer.h"

void HListener::Accept(std::shared_ptr<HPeer> const &peer) {
    // 放入 ec->holdItems 以确保 智能指针不自杀
    peer->Hold();

    // 设置 n 秒后断线( 触发 Close )
    peer->SetTimeoutSeconds(3);

    std::cout << "client(" << xx::ToString(peer->addr) << ") connected." << std::endl;
}
