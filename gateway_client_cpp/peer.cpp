#include "peer.h"
#include "client.h"

bool Peer::IsOpened(uint32_t const& serverId) const {
    return std::find(openServerIds.begin(), openServerIds.end(), serverId) != openServerIds.end();
}

Client &Peer::GetClient() {
    // 拿到服务上下文
    return *(Client *) ep;
}

void Peer::OnReceive() {
    // Disposed 判断变量
    EP::Ref<Item> alive(this);

    // 取出指针备用
    auto buf = recv.buf;
    auto end = recv.buf + recv.len;
    uint32_t dataLen = 0;
    uint32_t serverId = 0;

    // 确保包头长度充足
    while (buf + sizeof(dataLen) <= end) {
        // 取长度
        dataLen = *(uint32_t *) buf;

        // 长度异常则断线退出( 不含地址? 超长? 256k 不够可以改长 )
        if (dataLen < sizeof(serverId) || dataLen > 1024 * 256) {
            Dispose();
            return;
        }

        // 数据未接收完 就 跳出
        if (buf + sizeof(dataLen) + dataLen > end) break;

        // 跳到数据区开始调用处理回调
        buf += sizeof(dataLen);
        {
            // 取出地址( 手机上最好用 memcpy )
            serverId = *(uint32_t *) buf;

            // 包类型判断
            if (serverId == 0xFFFFFFFFu) {
                // 内部指令
                OnReceiveCommand(buf + sizeof(serverId), dataLen - sizeof(serverId));
            } else {
                // 普通包. serverId 打头
                OnReceivePackage(serverId, buf + sizeof(serverId), dataLen - sizeof(serverId));
            }

            // 如果当前类实例已自杀则退出
            if (!alive) return;
        }
        // 跳到下一个包的开头
        buf += dataLen;
    }

    // 移除掉已处理的数据( 将后面剩下的数据移动到头部 )
    recv.RemoveFront(buf - recv.buf);
}

void Peer::OnReceivePackage(uint32_t const &serverId, char *const &buf, size_t const &len) {
    // todo: 用 datareader 读出 serial. 剩余数据打包为 xx::Data
    //receivedPackages.emplace_back({serverId,, xx::Data(buf, len)});
}

void Peer::OnReceiveCommand(char *const &buf, size_t const &len) {
    // 要填充的变量
    std::string cmd;
    uint32_t serverId = 0;

    // 读取失败直接断开
    if (int r = xx::Read(buf, len, cmd, serverId)) {
        OnDisconnect(__LINE__);
        Dispose();
        return;
    }

    if (cmd == "open") {
        // serverId 放入白名单
        openServerIds.emplace_back(serverId);

        // 塞一条 serverId + 空数据 模拟 open 事件
        //receivedPackages.emplace_back(serverId, 0, xx::Data());
        // todo

    } else if (cmd == "close") {
        // serverId 从白名单移除
        openServerIds.erase(std::find(openServerIds.begin(), openServerIds.end(), serverId));
        if (openServerIds.empty()) {
            OnDisconnect(__LINE__);
            Dispose();
            return;
        }
        // close 可以不通知?
    }
    else {
        OnDisconnect(__LINE__);
        Dispose();
        return;
    }
}
