#include "dialer.h"
#include "server.h"
#include "speer.h"

inline Server &Dialer::GetServer() {
    // 拿到服务上下文
    return *(Server *) ep;
}

inline EP::TcpPeer_u Dialer::OnCreatePeer() {
    // 返回 Peer 类实例
    return xx::TryMakeU<SPeer>();
}

inline void Dialer::OnConnect(EP::TcpPeer_r const &peer_) {
    // 没连上
    if (!peer_) return;

    // 转为具体类型
    auto &&p = peer_.As<SPeer>();

    // 将 peer 放入容器
    auto &&s = GetServer();
    s.dps[serverId].second = p;

    // 继续填充一些依赖
    p->serverId = this->serverId;

    std::cout << "serverId: " << serverId << " connected." << std::endl;
}
