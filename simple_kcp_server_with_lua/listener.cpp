#include "listener.h"
#include "server.h"
#include "xx_logger.h"

Server &Listener::GetServer() {
    // 拿到服务上下文
    return *(Server *) &*ec;
}

void Listener::Accept(std::shared_ptr<CPeer> const &cp) {
    LOG_INFO("Listener Accept. ip = ", cp->addr);

    // 加持( Close 的时候 DelayUnhold )
    cp->Hold();

    // 设置自动断线超时时间
    cp->SetTimeoutSeconds(5);
}
