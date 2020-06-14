#include "client.h"
#include "dialer.h"
#include "header.h"

int Client::Run(double const &frameRate) {
    // 创建回收器
    xx::ScopeGuard sgCmdLine([&] {
        // 反注册交互指令( 消除对 Context 的引用计数的影响 )
        DisableCommandLine();
        // 清理成员变量( 消除对 Context 的引用计数的影响 )
        dialer.reset();
        peer.reset();
    });
    // 初始化拨号器
    xx::MakeTo(dialer, shared_from_this());
    // 添加默认拨号地址
    dialer->AddAddress("127.0.0.1", 10000);
    // 注册交互指令
    EnableCommandLine();

    cmds["send"] = [this](std::vector<std::string> const &args) {
        if (args.size() < 2) {
            std::cout << "send: miss data" << std::endl;
        } else if (args[1].size() > std::numeric_limits<decltype(Header::len)>::max()) {
            std::cout << "send: data's size > max len" << std::endl;
        } else if (!peer) {
            std::cout << "send: no peer" << std::endl;
        } else {
            peer->SendPackage(args[1].data(), args[1].size());
            std::cout << "send: success" << std::endl;
        }
    };

    cmds["quit"] = [this](auto args) {
        running = false;
    };

    cmds["exit"] = cmds["quit"];

    // 开始循环
    return this->EP::Context::Run(frameRate);
}

int Client::FrameUpdate() {
    // 自动拨号 & 重连逻辑
    // 如果未创建连接并且拨号器没有正在拨号, 就开始拨号
    if (!peer && !dialer->Busy()) {
        // 超时时间 2 秒
        dialer->DialSeconds(2);
    }
    return 0;
}
