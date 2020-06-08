#include "glistener.h"
#include "server.h"
#include "gpeer.h"

EP::TcpPeer_u GListener::OnCreatePeer() {
    // 返回 Peer 类实例
    return xx::TryMakeU<GPeer>();
}

void GListener::OnAccept(EP::TcpPeer_r const &peer_) {
    // 没连上
    if (!peer_) return;

    // 转为本来的类型
    auto&& p = peer_.As<GPeer>();

    // 设置使用 匿名处理类并将 peer 放入相应容器
    p->SetAPHandler();
}
