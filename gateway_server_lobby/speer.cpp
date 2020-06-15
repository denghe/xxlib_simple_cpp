#include "server.h"
#include "speer.h"

bool SPeer::Close(int const& reason) {
    // 防重入( 同时关闭 fd )
    if (!this->Peer::Close(reason)) return false;
    // 从容器移除以减持( 如果有放入的话 )
    if (id) {
        GetServer().sps.erase(id);
    }
    // 延迟减持
    DelayUnhold();
    return true;
}

void SPeer::ReceivePackage(char *const &buf, size_t const &len) {
    // todo:
    // 服务之间正常通信
}

void SPeer::ReceiveFirstPackage(char *const &buf, size_t const &len) {
    // 解析首包. 内容应该由 string + uint 组成
    std::string cmd;
    uint32_t serverId = 0;
    xx::DataReader dr(buf, len);

    // 读取失败: 断线退出
    if (dr.Read(cmd, serverId)) {
        Close(__LINE__);
        return;
    }

    // 前置检查失败: 断线退出
    if (cmd != "serverId" || !serverId) {
        Close(__LINE__);
        return;
    }

    // for easy use
    auto &&sps = GetServer().sps;

    // 如果 serverId 已存在: 断线退出
    if (sps.find(serverId) != sps.end()) {
        Close(__LINE__);
        return;
    }

    // 放入相应容器( 通常这部分和相应的负载均衡或者具体游戏配置密切相关 )
    sps[serverId] = xx::As<SPeer>(shared_from_this());
    id = serverId;

    // 设置不超时
    SetTimeoutSeconds(0);
}
