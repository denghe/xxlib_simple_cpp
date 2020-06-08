#include "glistener.h"
#include "gpeer.h"

EP::TcpPeer_u GListener::OnCreatePeer() {
    // 返回 Peer 类实例
    return xx::TryMakeU<GPeer>();
}

void GListener::OnAccept(EP::TcpPeer_r const &p) {
    // 连上了
    if (p) {
        // 设置超时时长( 经验数值 )避免 fd 泄露
        p->SetTimeoutSeconds(5);
    }
}
