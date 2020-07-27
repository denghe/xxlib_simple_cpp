#include "speer.h"
#include "server.h"
#include "cpeer.h"
#include "xx_data_rw.h"

bool SPeer::Close(int const& reason, char const* const& desc) {
    // 防重入( 同时关闭 fd )
    if (!this->Peer::Close(reason)) return false;
    ec->Log<2>("SPeer Close. serverId = ", serverId, ", reason = ", reason, ", desc = ", desc);
    // 从所有 client peers 里的白名单中移除
    for (auto &&kv : GetServer().cps) {
        // 不管有没有, 试着 从 client peer 的 已open id列表 擦除该 服务id
        kv.second->serverIds.erase(serverId);
        // 下发 close( 如果全都没了, 则会导致 client 无法发送任何数据过来，自然超时断掉 )
        kv.second->SendCommand("close", serverId);
    }
    // 从 dps 移除以减持
    GetServer().dps[serverId].second.reset();
    // 延迟减持
    DelayUnhold();
    return true;
}

void SPeer::ReceivePackage(char *const &buf, size_t const &len) {
    // 读出 clientId
    auto&& clientId = *(uint32_t *) buf;
    // 如果找到 client peer, 则转发
    if(auto &&cp = TryGetCPeer(clientId)) {
        ec->Log<5>("SPeer ReceivePackage. serverId = ", serverId, ", clientId = ", clientId, ", buf len = ", len);
        // 篡改 clientId 为 serverId
        *(uint32_t *) buf = serverId;

        // 从 长度包头 处开始，直接转发
        cp->Send(buf - 4, len + 4);
    }
    else {
        // 没找到：输出点日志
        ec->Log<4>("SPeer ReceivePackage TryGetCPeer failed. serverId = ", serverId, ", clientId = ", clientId, ", buf len = ", len);
    }
}

void SPeer::ReceiveCommand(char *const &buf, size_t const &len) {
    // for easy use
    auto&& s = GetServer();

    // 创建一个数据读取器
    xx::DataReader dr(buf, len);

    // 试读取 cmd 字串. 失败直接断开
    std::string cmd;
    if (int r = dr.Read(cmd)) {
        ec->Log<2>("SPeer ReceiveCommand Read failed. serverId = ", serverId, ", r = ", r);
        Close(__LINE__, __FILE__);
        return;
    }

    // 公用 client id 容器
    uint32_t clientId = 0;

    if (cmd == "open") {                        // 向客户端开放 serverId. 参数: clientId
        // 试读出 clientId. 失败直接断开
        if (int r = dr.Read(clientId)) {
            ec->Log<2>("SPeer ReceiveCommand Read 'open' failed. serverId = ", serverId, ", r = ", r);
            Close(__LINE__, __FILE__);
            return;
        }
        // 如果找到 client peer, 则转发
        if (auto &&cp = TryGetCPeer(clientId)) {
            ec->Log<3>("SPeer ReceiveCommand 'open'. serverId = ", serverId, ", clientId = ", clientId);
            // 放入白名单
            cp->serverIds.emplace(serverId);
            // 下发 open
            cp->SendCommand("open", serverId);
        }
        else {
            // 未找到
            ec->Log<3>("SPeer ReceiveCommand 'open'. TryGetCPeer failed. serverId = ", serverId, ", clientId = ", clientId);
        }
    } else if (cmd == "close") {                // 关端口. 参数: clientId
        // 试读出 clientId. 失败直接断开
        if (int r = dr.Read(clientId)) {
            ec->Log<2>("SPeer ReceiveCommand Read 'close' failed. serverId = ", serverId, ", r = ", r);
            Close(__LINE__, __FILE__);
            return;
        }
        // 如果找到 client peer, 则转发
        if (auto &&cp = TryGetCPeer(clientId)) {
            ec->Log<3>("SPeer ReceiveCommand 'close'. serverId = ", serverId, ", clientId = ", clientId);
            // 从白名单移除
            cp->serverIds.erase(serverId);
            // 下发 close
            cp->SendCommand("close", serverId);
        }
        else {
            // 未找到
            ec->Log<3>("SPeer ReceiveCommand 'close'. TryGetCPeer failed. serverId = ", serverId, ", clientId = ", clientId);
        }
    } else if (cmd == "kick") {                 // 踢玩家下线. 参数: clientId, delayMS
        // 试读出参数
        int64_t delayMS = 0;
        if (int r = dr.Read(clientId, delayMS)) {
            ec->Log<2>("SPeer ReceiveCommand Read 'kick' failed. serverId = ", serverId, ", r = ", r);
            Close(__LINE__, __FILE__);
            return;
        }
        // 如果没找到 或已断开 则返回，忽略错误
        auto &&cp = TryGetCPeer(clientId);
        if (!cp) {
            // 未找到
            ec->Log<3>("SPeer ReceiveCommand 'kick'. TryGetCPeer failed. serverId = ", serverId, ", clientId = ", clientId);
        }

        // 延迟踢下线
        if (delayMS) {
            ec->Log<3>("SPeer ReceiveCommand 'kick' DelayClose. serverId = ", serverId, ", clientId = ", clientId, ", delayMS = ", delayMS);
            // 下发一个 close 指令以便 client 收到后直接主动断开, 响应会比较快速
            cp->SendCommand("close", (uint32_t)0);
            // 会立即从 cps 移除并向白名单 serverIds 对应 peer 广播断开通知. 这之后不再处理收到的消息, 直到超时自杀
            cp->DelayClose((double) delayMS / 1000);
        } else {
            ec->Log<3>("SPeer ReceiveCommand 'kick' Close. serverId = ", serverId, ", clientId = ", clientId);
            // 立刻踢下线
            cp->Close(__LINE__, __FILE__);
        }
    } else {
        // 收到没有处理函数对应的指令
        ec->Log<2>("SPeer ReceiveCommand unhandled cmd. serverId = ", serverId, ", cmd = ", cmd);
        Close(__LINE__, __FILE__);
    }
}

CPeer *SPeer::TryGetCPeer(uint32_t const &clientId) {
    auto &&cps = GetServer().cps;
    // 根据 client id 找。找不到就返回空指针
    auto &&iter = cps.find(clientId);
    if (iter == cps.end()) return nullptr;
    return &*iter->second;
}
