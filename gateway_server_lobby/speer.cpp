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

//        // 通过移交 fd 保持连接不断开 的方式来切换 peer.
//        // 这样一来 可以做功能单一的 "首包通信专用 peer", 还能简化掉 OnReceiveFirstPackage 事件
//        // 首包应 一来一回，确保对方 ensure 才继续通信，避免出现 除了首包之外还有残留数据 导致无法切 peer 的尴尬
//        // 例如 对方发送它的 serverId 过来，这边也回应一句自己的 serverId, 还能起到校验 config 的作用.
//        auto sp = CreateByFD<SPeer>();
//        sp->id = serverId;
//        sps[serverId] = sp;
//        sp.SendServerId( config.serverId );
//        Dispose();
//        return;

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
