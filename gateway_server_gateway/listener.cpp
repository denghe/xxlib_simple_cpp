#include "listener.h"
#include "server.h"
#include "cpeer.h"
#include "speer.h"
#include "config.h"

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

    // 引用到 server 的dps 备用
    auto &&s = GetServer();

    // 检查是否已经与 0 号服务建立了连接. 如果没有，则直接断开 退出
    auto&& s0 = s.dps[0].second.Lock();
    if(!s0) {
        peer_->Dispose();
        return;
    }

    // 转为具体类型
    auto &&cp = peer_.As<CPeer>();

    // 填充自增id
    cp->clientId = ++s.cpeerAutoId;

    // 放入容器
    s.cps[cp->clientId] = cp;

    // 设置自动断线超时时间
    cp->SetTimeoutSeconds(config.clientTimeoutSeconds);

    // 向默认服务发送 accept 通知
    s0->SendCommand_Accept(cp->clientId, EP::AddressToString(cp->addr));
}
