#include "dialer.h"
#include "server.h"
#include "config.h"

Server &Dialer::GetServer() {
    // 拿到服务上下文
    return *(Server *) &*ec;
}

void Dialer::Connect(std::shared_ptr<LPeer> const &sp) {
    // 没连上
    if (!sp) return;

    // 加持
    sp->Hold();

    // 将 peer 放入容器
    GetServer().lobbyPeer = sp;

    // 随便填个值避免触发 ReceiveFirstPackage
    // 因为是连向 lobby, 由这边发首包, 成功 lobby 无回复. 失败会被断开
    sp->id = -1;

    // 向 lobby 发送首包: LEN + “serverId” + serverId
    xx::Data d;
    d.Reserve(32);
    d.len = sizeof(uint32_t);
    xx::Write(d, "serverId", config.serverId);
    *(uint32_t *) d.buf = (uint32_t) (d.len - sizeof(uint32_t));
    sp->Send(std::move(d));
}
