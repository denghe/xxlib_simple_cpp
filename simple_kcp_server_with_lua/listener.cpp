#include "listener.h"
#include "server.h"
#include "xx_logger.h"

Server &Listener::GetServer() {
    // 拿到服务上下文
    return *(Server *) &*ec;
}

void Listener::Accept(std::shared_ptr<CPeer> const &cp) {
    LOG_INFO("Listener Accept. ip = ", cp->addr);
    // 引用到 server 备用
    auto &&s = GetServer();

    // 加持
    cp->Hold();

    // 放入容器
    s.cps[&*cp] = cp;

    // 设置自动断线超时时间
    cp->SetTimeoutSeconds(15);
}
