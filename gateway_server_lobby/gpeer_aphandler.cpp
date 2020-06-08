#include "gpeer_aphandler.h"
#include "gpeer.h"
#include "server.h"

APHandler::APHandler(GPeer &peer, uint32_t const &id) : PHandler(peer, id) {
    // 放入容器
    peer.GetServer().aps[id] = &peer;
}

APHandler::~APHandler() {
    // 如果是因 server 析构导致执行到此, 则 server 派生层 成员 已析构, 不可访问. 短路退出
    if (!peer.GetServer().running) return;

    // 从容器移除
    peer.GetServer().aps.erase(id);
}

void APHandler::OnReceivePackage(char *const &buf, size_t const &len) {
    // 匿名阶段不应该收到普通包. 收到就直接掐
    peer.OnDisconnect(__LINE__);
    peer.Dispose();
}

void APHandler::OnReceiveCommand(char *const &buf, size_t const &len) {
    // 引用到 server 备用
    auto &&s = peer.GetServer();

    // 创建一个数据读取器
    xx::DataReader dr(buf, len);

    // 指令名
    std::string cmd;
    do {
        // 试读取 cmd 字串. 失败直接断开
        if (int r = dr.ReadLimit<64>(cmd)) break;

        if (cmd == "gatewayId") {
            // 指令参数
            uint32_t gatewayId = 0;

            // 试读出 指令参数. 失败直接断开
            if (int r = dr.Read(gatewayId)) break;

            // 检查是否存在. 如果已存在就断开. 不顶下线
            if (s.gps.find(gatewayId) != s.gps.end()) break;

            // 切换处理程序 并调整所在容器( 会导致当前 phandler 析构 )
            peer.SetGPHandler(gatewayId);
            return;
        } else if (cmd == "serverId") {
            // 指令参数
            uint32_t serverId = 0;

            // 试读出 指令参数. 失败直接断开
            if (int r = dr.Read(serverId)) break;

            std::cout << "serverId = " << serverId << std::endl;

            // 检查是否存在. 如果已存在就断开. 不顶下线
            if (s.sps.find(serverId) != s.sps.end()) break;

            // 切换处理程序 并调整所在容器( 会导致当前 phandler 析构 )
            peer.SetSPHandler(serverId);
            return;
        }
    } while (0);

    // 触发断线事件并自杀
    peer.OnDisconnect(__LINE__);
    peer.Dispose();
}
