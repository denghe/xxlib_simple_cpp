#include "listener.h"
#include "cpeer.h"

Server &Listener::GetServer() {
    // 拿到服务上下文
    return *(Server *) ep;
}

EP::TcpPeer_u Listener::OnCreatePeer() {
    // 返回 Peer 类实例
    return xx::TryMakeU<CPeer>();
}

void Listener::OnAccept(EP::TcpPeer_r const &peer_) {
    // 没连上
    if (!peer_) return;

    // 转为具体类型
    auto &&p = peer_.As<CPeer>();

    // todo:
}
