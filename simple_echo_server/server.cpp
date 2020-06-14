#include "server.h"
#include "listener.h"
#include "config.h"

int Server::Run(double const &frameRate) {
    // 建立回收手段
    xx::ScopeGuard sgListener([&]{
        // 清理监听器( 消除对 Context 的引用计数的影响 )
        listener.reset();
    });
    // 按配置的端口创建监听器
    xx::MakeTo(listener, shared_from_this());
    if(listener->Listen(config.listenPort)) {
        std::cout << "listen to port " << std::to_string(config.listenPort) << " failed." << std::endl;
        return __LINE__;
    }
    // 进入循环
    return this->EP::Context::Run(frameRate);
}
