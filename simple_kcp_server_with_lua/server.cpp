#include "server.h"
#include "listener.h"
#include "xx_logger.h"

int Server::Run() {
    // 初始化回收sg, 以便退出 Run 时清理会加持宿主的成员
    xx::ScopeGuard sg1([&]{
        DisableCommandLine();
        listener.reset();
        holdItems.clear();
        assert(shared_from_this().use_count() == 2);
    });

    // 初始化监听器
    xx::MakeTo(listener, shared_from_this());
    // 如果监听失败则输出错误提示并退出
    if (int r = listener->Listen(12333)) {
        LOG_ERROR("listen to port ", 12333, "failed.");
        return r;
    }

    // 正式进入 epoll wait 循环
    return this->EP::Context::Run();
}

int Server::FrameUpdate() {
    return 0;
}
