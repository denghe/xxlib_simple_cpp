#include "aphandler.h"
#include "peer.h"
#include "server.h"

void APHandler::OnReceivePackage(char *const &buf, size_t const &len) {
    // 匿名阶段不应该收到普通包. 收到就直接掐
    peer.Dispose();
}

void APHandler::OnReceiveCommand(char *const &buf, size_t const &len) {
    // 创建一个数据读取器
    xx::DataReader dr(buf, len);

    // 试读取 cmd 字串. 失败直接断开
    std::string cmd;
    if (int r = dr.ReadLimit<64>(cmd)) {
        Dispose();
        return;
    }

    // 引用到 server 备用
    auto &&s = GetServer();

    // 公用 id 容器
    uint32_t id = 0;

    if (cmd == "gatewayId") {
        // 试读出 gatewayId. 失败直接断开
        if (int r = dr.Read(id)) {
            Dispose();
            return;
        }
        // todo: 检查是否存在. 如果已存在就断开. 不顶下线
        //s.gps[id] = peer;
        peer.SetGPHandler(id);          // 小心: 这行代码会导致当前类立即析构
    } else if (cmd == "serverId") {
        // 试读出 serverId. 失败直接断开
        if (int r = dr.Read(id)) {
            Dispose();
            return;
        }
        // todo: 检查是否存在. 如果已存在就断开. 不顶下线
        //s.sps[id] = peer;
        peer.SetSPHandler(id);          // 小心: 这行代码会导致当前类立即析构
    } else {
        // 未知指令
        Dispose();                      // 小心: 这行代码会导致当前类立即析构
    }
}

void APHandler::OnDisconnect(int const &reason) {
    // 从容器移除
    GetServer().aps.erase(id);
}
