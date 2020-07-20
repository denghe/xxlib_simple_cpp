#include "server.h"
#include "listener.h"
#include "config.h"

int Server::Run() {
    // 初始化回收sg, 以便退出 Run 时清理会加持宿主的成员
    xx::ScopeGuard sg([&]{
        listener.reset();
        DisableCommandLine();
        auto c = shared_from_this().use_count();
        assert(c == 2);
    });
    // 按配置的端口创建监听器
    xx::MakeTo(listener, shared_from_this());
    if(listener->Listen(config.listenPort)) {
        std::cout << "listen to port " << std::to_string(config.listenPort) << " failed." << std::endl;
        return __LINE__;
    }
    // 进入循环
    return this->EP::Context::Run();
}
