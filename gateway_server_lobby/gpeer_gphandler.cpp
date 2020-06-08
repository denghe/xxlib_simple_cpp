#include "gpeer_gphandler.h"
#include "gpeer.h"
#include "server.h"

GPHandler::GPHandler(Peer &peer, uint32_t const &id) : PHandler(peer, id) {
    // 放入容器
    peer.GetServer().gps[id] = &peer;
}

GPHandler::~GPHandler() {
    // 如果是因 server 析构导致执行到此, 则 server 派生层 成员 已析构, 不可访问. 短路退出
    if (!peer.GetServer().running) return;

    // 从容器移除
    peer.GetServer().gps.erase(id);
}

void GPHandler::OnReceivePackage(char *const &buf, size_t const &len) {
    // todo: 读取 clientId 转发到 相应虚拟 peer
}

void GPHandler::OnReceiveCommand(char *const &buf, size_t const &len) {
    // 引用到 server 备用
    auto &&s = peer.GetServer();

    // 创建一个数据读取器
    xx::DataReader dr(buf, len);

    // 指令名
    std::string cmd;
    do {
        // 试读取 cmd 字串. 失败直接断开
        if (int r = dr.ReadLimit<64>(cmd)) break;

        if (cmd == "accept") {
            // 指令参数
            uint32_t clientId = 0;
            std::string ip;

            // 试读出 指令参数. 失败直接断开
            if (int r = dr.Read(clientId, ip)) break;

            // todo: 创建虚拟 peer
            return;
        } else if (cmd == "disconnect") {
            // 指令参数
            uint32_t clientId = 0;

            // 试读出 指令参数. 失败直接断开
            if (int r = dr.Read(clientId)) break;

            // todo: 干掉虚拟 peer
            return;
        }
    } while (0);

    // 触发断线事件并自杀
    peer.OnDisconnect(__LINE__);
    peer.Dispose();
}
