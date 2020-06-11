#include "speer.h"
#include "server.h"
#include "cpeer.h"
#include "xx_datareader.h"

void SPeer::SendCommand_GatewayId(uint32_t const &gatewayId) {
    SendCommand("gatewayId", gatewayId);
}

void SPeer::SendCommand_Accept(uint32_t const &clientId, std::string const &ip) {
    SendCommand("accept", clientId, ip);
}

void SPeer::SendCommand_Disconnect(uint32_t const &clientId) {
    SendCommand("disconnect", clientId);
}

void SPeer::OnReceivePackage(char *const &buf, size_t const &len) {
    // 读出 clientId
    auto&& clientId = *(uint32_t *) buf;

    //std::cout << "recv data from clientId = " << clientId << ", len = " << len << std::endl;

    // 如果没找到 client peer, 则忽略并返回
    if(auto &&cp = TryGetCPeer(clientId)) {
        // 篡改 clientId 为 serverId
        *(uint32_t *) buf = serverId;

        // 从 长度包头 处开始，直接转发
        cp->Send(buf - 4, len + 4);
    }
}

void SPeer::OnReceiveCommand(char *const &buf, size_t const &len) {
    // 创建一个数据读取器
    xx::DataReader dr(buf, len);

    // 试读取 cmd 字串. 失败直接断开
    std::string cmd;
    if (int r = dr.ReadLimit<64>(cmd)) {
        OnDisconnect(__LINE__);
        Dispose();
        return;
    }

    // 公用 client id 容器
    uint32_t clientId = 0;

    if (cmd == "open") {                        // 向客户端开放 serverId. 参数: clientId
        // 试读出 clientId. 失败直接断开
        if (int r = dr.Read(clientId)) {
            OnDisconnect(__LINE__);
            Dispose();
            return;
        }
        // 如果没找到 client peer, 则忽略
        if (auto &&cp = TryGetCPeer(clientId)) {
            // 放入白名单
            cp->serverIds.emplace(serverId);

            // 下发 open
            cp->SendCommand_Open(serverId);
        }
    } else if (cmd == "close") {                // 关端口. 参数: clientId
        // 试读出 clientId. 失败直接断开
        if (int r = dr.Read(clientId)) {
            OnDisconnect(__LINE__);
            Dispose();
            return;
        }
        // 如果没找到 client peer, 则忽略
        if (auto &&cp = TryGetCPeer(clientId)) {
            // 从白名单移除
            cp->serverIds.erase(serverId);

            // 下发 close
            cp->SendCommand_Close(serverId);
        }
    } else if (cmd == "kick") {                 // 踢玩家下线. 参数: clientId, delayMS
        // 试读出参数
        int64_t delayMS = 0;
        if (int r = dr.Read(clientId, delayMS)) {
            OnDisconnect(__LINE__);
            Dispose();
            return;
        }
        // 如果没找到 或已断开 则返回，忽略错误
        auto &&cp = TryGetCPeer(clientId);
        if (!cp) return;

        // 延迟踢下线
        if (delayMS) {
            // 下发一个 close 指令以便 client 收到后直接主动断开, 响应会比较快速
            cp->SendCommand_Close(0);
            // 利用超时来 Dispose
            cp->SetTimeoutSeconds((double) delayMS / 1000);
            // 主动调用 OnDisconnect 事件代码：从 cps 移除并向白名单 serverIds 对应 peer 广播断开通知
            cp->OnDisconnect(1);
        } else {
            // 立刻踢下线，触发 OnDisconnect
            cp->Dispose();
        }
    } else {                                    // 未知指令
        OnDisconnect(__LINE__);
        Dispose();
        return;
    }
}

void SPeer::OnDisconnect(int const &reason) {
    // 从所有 client peers 里的白名单中移除
    for (auto &&kv : GetServer().cps) {
        // 下面是连贯操作，可先拿到裸指针
        auto &&cp = kv.second.Lock();

        // 不管有没有, 试着 从 client peer 的 已open id列表 擦除该 服务id
        cp->serverIds.erase(serverId);

        // 下发 close( 如果全都没了, 则会导致 client 无法发送任何数据过来，自然超时断掉 )
        cp->SendCommand_Close(serverId);
    }
}

CPeer *SPeer::TryGetCPeer(uint32_t const &clientId) {
    auto &&cps = GetServer().cps;
    auto &&iter = cps.find(clientId);
    if (iter == cps.end()) return nullptr;
    return iter->second.Lock();
}
