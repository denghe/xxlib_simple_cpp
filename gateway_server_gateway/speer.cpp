#include "speer.h"
#include "server.h"
#include "cpeer.h"
#include "xx_datareader.h"

bool SPeer::Close(int const& reason) {
    // close fd 解绑 并触发 OnDisconnect
    if (this->Peer::Close(reason)) {
        // 从所有 client peers 里的白名单中移除
        for (auto &&kv : GetServer().cps) {
            // 不管有没有, 试着 从 client peer 的 已open id列表 擦除该 服务id
            kv.second->serverIds.erase(serverId);
            // 下发 close( 如果全都没了, 则会导致 client 无法发送任何数据过来，自然超时断掉 )
            kv.second->SendCommand("close", serverId);
        }
        // 从 dps 移除, 延迟自杀
        GetServer().dps[serverId].second.reset();
        DelayUnhold();
        return true;
    }
    return false;
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
        cp->Send({buf - 4, len + 4});
    }
}

void SPeer::OnReceiveCommand(char *const &buf, size_t const &len) {
    // for easy use
    auto&& s = GetServer();

    // 创建一个数据读取器
    xx::DataReader dr(buf, len);

    // 试读取 cmd 字串. 失败直接断开
    std::string cmd;
    if (int r = dr.ReadLimit<64>(cmd)) {
        Close(__LINE__);
        return;
    }

    // 公用 client id 容器
    uint32_t clientId = 0;

    if (cmd == "open") {                        // 向客户端开放 serverId. 参数: clientId
        // 试读出 clientId. 失败直接断开
        if (int r = dr.Read(clientId)) {
            Close(__LINE__);
            return;
        }
        // 如果没找到 client peer, 则忽略
        if (auto &&cp = TryGetCPeer(clientId)) {
            // 放入白名单
            cp->serverIds.emplace(serverId);
            // 下发 open
            cp->SendCommand("open", serverId);
        }
    } else if (cmd == "close") {                // 关端口. 参数: clientId
        // 试读出 clientId. 失败直接断开
        if (int r = dr.Read(clientId)) {
            Close(__LINE__);
            return;
        }
        // 如果没找到 client peer, 则忽略
        if (auto &&cp = TryGetCPeer(clientId)) {
            // 从白名单移除
            cp->serverIds.erase(serverId);
            // 下发 close
            cp->SendCommand("close", serverId);
        }
    } else if (cmd == "kick") {                 // 踢玩家下线. 参数: clientId, delayMS
        // 试读出参数
        int64_t delayMS = 0;
        if (int r = dr.Read(clientId, delayMS)) {
            Close(__LINE__);
            return;
        }
        // 如果没找到 或已断开 则返回，忽略错误
        auto &&cp = TryGetCPeer(clientId);
        if (!cp) return;

        // 延迟踢下线
        if (delayMS) {
            // 下发一个 close 指令以便 client 收到后直接主动断开, 响应会比较快速
            cp->SendCommand("close", (uint32_t)0);
            // 利用超时来 Close. 会立即 OnDisconnect 并从 cps 移除并向白名单 serverIds 对应 peer 广播断开通知
            cp->DelayClose((double) delayMS / 1000);
        } else {
            // 立刻踢下线，触发 OnDisconnect
            cp->Close(__LINE__);
        }
    } else {                                    // 未知指令
        Close(__LINE__);
    }
}

void SPeer::OnDisconnect(int const &reason) {
    std::cout << "SPeer OnDisconnect. serverId = "<< serverId <<", reason = " << reason << std::endl;
}

CPeer *SPeer::TryGetCPeer(uint32_t const &clientId) {
    auto &&cps = GetServer().cps;
    // 根据 client id 找。找不到就返回空指针
    auto &&iter = cps.find(clientId);
    if (iter == cps.end()) return nullptr;
    return &*iter->second;
}
