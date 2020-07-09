#include "client.h"
#include "dialer.h"
#include "header.h"

int Client::Run() {
    // 初始化回收sg, 以便退出 Run 时清理会加持宿主的成员
    xx::ScopeGuard sg([&] {
        dialer.reset();
        peer.reset();
        DisableCommandLine();
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
    return this->EP::Context::Run();
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
