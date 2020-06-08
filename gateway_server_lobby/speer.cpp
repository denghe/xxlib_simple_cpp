#include "server.h"
#include "speer.h"

SPeer::~SPeer() {
    // 如果是因 server 析构导致执行到此, 则 server 派生层 成员 已析构, 不可访问. 短路退出
    if (!GetServer().running) return;

    // 从容器移除( 如果有放入的话 )
    if (id) {
        GetServer().sps.erase(id);
    }
}

void SPeer::OnReceivePackage(char *const &buf, size_t const &len) {
    // todo:
    // 服务之间正常通信
}

void SPeer::OnReceiveFirstPackage(char *const &buf, size_t const &len) {
    // 解析首包. 内容应该由 string + uint 组成
    std::string cmd;
    uint32_t serverId = 0;
    xx::DataReader dr(buf, len);

    do {
        // 读取失败: 断线退出
        if (dr.Read(cmd, serverId)) break;

        // 前置检查失败: 断线退出
        if (cmd != "serverId" || !serverId) break;

        // for easy use
        auto &&sps = GetServer().sps;

        // 如果 serverId 已存在: 断线退出
        if (sps.find(serverId) != sps.end()) break;

        // 放入相应容器( 通常这部分和相应的负载均衡或者具体游戏配置密切相关 )
        sps[serverId] = this;
        id = serverId;

        // 设置不超时
        SetTimeoutSeconds(0);

        return;
    } while (0);

    // 触发断线事件并自杀
    OnDisconnect(__LINE__);
    Dispose();
}
