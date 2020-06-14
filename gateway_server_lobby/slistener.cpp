#include "slistener.h"

void SListener::OnAccept(std::shared_ptr<SPeer> const &p) {
    // 没连上
    if (!p) return;

    // 设置超时时长( 经验数值 )避免 fd 泄露
    p->SetTimeoutSeconds(5);

    // 持有
    p->Hold();
}
