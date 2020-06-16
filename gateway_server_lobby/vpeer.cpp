#include "vpeer.h"
#include "gpeer.h"

VPeerCB::VPeerCB(std::shared_ptr<VPeer> const &vpeer, int const &serial,
                 std::function<void(char const *const &buf, size_t const &len)> &&cbfunc, double const &timeoutSeconds)
        : EP::Timer(vpeer->ec, -1), serial(serial), func(std::move(cbfunc)) {
    SetTimeoutSeconds(timeoutSeconds);
}

void VPeerCB::Timeout() {
    // 模拟超时
    func(nullptr, 0);
    // 从容器移除
    vpeer->callbacks.erase(serial);
    // 延迟自杀
    DelayUnhold();
}

VPeer::VPeer(std::shared_ptr<GPeer> const &gatewayPeer, uint32_t const &clientId)
        : EP::Item(gatewayPeer->ec, -1), gatewayPeer(gatewayPeer), clientId(clientId) {
    // 通过网关向客户端发 open 指令
    gatewayPeer->SendTo(0xFFFFFFFFu, "open", clientId);
}

int VPeer::SendPush(char const *const &buf, size_t const &len) const {
    // 推送性质的包, serial == 0
    return SendResponse(0, buf, len);
}

int VPeer::SendResponse(int32_t const &serial, char const *const &buf, size_t const &len) const {
    // 准备发包填充容器
    xx::Data d(len + 13);
    // 跳过包头
    d.len = sizeof(uint32_t);
    // 写 要发给谁
    d.WriteFixed(clientId);
    // 写序号
    d.WriteVarIntger(serial);
    // 写数据
    d.WriteBuf(buf, len);
    // 填包头
    *(uint32_t *) d.buf = (uint32_t) (d.len - sizeof(uint32_t));
    // 发包并返回
    return gatewayPeer->Send(std::move(d));
}

int VPeer::SendRequest(char const *const &buf, size_t const &len,
                       std::function<void(char const *const &buf, size_t const &len)> &&cbfunc,
                       double const &timeoutSeconds) {
    // 产生一个序号. 在正整数范围循环自增( 可能很多天之后会重复 )
    autoIncSerial = (autoIncSerial + 1) & 0x7FFFFFFF;
    // 创建一个 带超时的回调
    auto &&cb = xx::Make<VPeerCB>(xx::As<VPeer>(shared_from_this()), autoIncSerial, std::move(cbfunc), timeoutSeconds);
    cb->Hold();
    // 以序列号建立cb的映射
    callbacks[autoIncSerial] = cb;
    // 发包并返回( 请求性质的包, 序号为负数 )
    return SendResponse(-autoIncSerial, buf, len);
}

bool VPeer::Close(int const &reason) {
    // 防范重入
    if (clientId == 0xFFFFFFFFu) return false;
    // 触发断线事件
    OnDisconnect(reason);
    // 如果物理 peer 没断:
    if (gatewayPeer->Alive()) {
        // 通过网关向客户端发 close 指令
        gatewayPeer->SendTo(0xFFFFFFFFu, "close", clientId);
        // 从容器移除自己
        gatewayPeer->vpeers.erase(clientId);
    }
    // 触发所有已存在回调（ 模拟立刻超时 ）
    for (auto &&iter : callbacks) {
        // 模拟超时回调
        iter.second->func(nullptr, 0);
        // 延迟自杀
        iter.second->DelayUnhold();
    }
    // 从容器中移除所有回调( 相互减持 )
    callbacks.clear();

    // 延迟自杀
    DelayUnhold();
    // 防范重入
    clientId = 0xFFFFFFFFu;
    return true;
}


void VPeer::Receive(char const *const &buf, size_t const &len) {
    // 试读出序号. 出错直接断开退出
    int serial = 0;
    xx::DataReader dr(buf, len);
    if (dr.Read(serial)) {
        Close(__LINE__);
        return;
    }

    // 根据序列号的情况分性质转发
    if (serial == 0) {
        ReceivePush(buf + dr.offset, len - dr.offset);
    } else if (serial > 0) {
        ReceiveResponse(serial, buf + dr.offset, len - dr.offset);
    } else {
        // -serial: 将 serial 转为正数
        ReceiveRequest(-serial, buf + dr.offset, len - dr.offset);
    }
}

void VPeer::ReceiveResponse(uint32_t const &serial, char const *const &buf, size_t const &len) {
    // 根据序号定位到 cb. 找不到可能是超时或发错?
    auto &&iter = callbacks.find(serial);
    if (iter == callbacks.end()) return;
    // 先取出来并从回调容器移除( 防止 callback 中有自杀行为导致中间逻辑不方便判断 )
    auto &&cb = std::move(iter->second);
    callbacks.erase(iter);
    // 执行后延迟自杀
    cb->func(buf, len);
    cb->DelayUnhold();
}


void VPeer::ReceivePush(char const *const &buf, size_t const &len) {
    // 模拟某协议解包
    std::string txt;
    if (xx::Read(buf, len, txt)) {
        Close(__LINE__);
        return;
    }
    std::cout << "vpeer: " << clientId << " recv push: " << txt << std::endl;
    // todo
}

void VPeer::ReceiveRequest(uint32_t const &serial, char const *const &buf, size_t const &len) {
    // 模拟某协议解包
    std::string txt;
    if (xx::Read(buf, len, txt)) {
        Close(__LINE__);
        return;
    }
    std::cout << "vpeer: " << clientId << " recv request: " << txt << std::endl;

    if (txt == "auth") {
        // 模拟构造回包: 服务类型 + serverId
        xx::Data d;
        xx::Write(d, "lobby", (uint32_t) 0);
        SendResponse(serial, d.buf, d.len);
        // 通知 gateway open 指定 serverId?
    } else if (txt == "info") {
        // 模拟构造回包
        xx::Data d;
        xx::Write(d, "username:xxxxx coin:123123123");
        SendResponse(serial, d.buf, d.len);
    } else {
        // 未知包?
        Close(__LINE__);
    }
}

void VPeer::OnDisconnect(int const &reason) {
    std::cout << "vpeer: " << clientId << " disconnected. reason: " << reason << std::endl;
}


void VPeer::SwapClientId(std::shared_ptr<VPeer> const& o) {
    if (!o || clientId == o->clientId || !gatewayPeer) return;
    if (clientId == 0xFFFFFFFFu) {
        gatewayPeer->vpeers[o->clientId] = xx::As<VPeer>(shared_from_this());
        clientId = o->clientId;
        o->clientId = 0xFFFFFFFFu;
    }
    else if(o->clientId == 0xFFFFFFFFu) {
        gatewayPeer->vpeers[clientId] = o;
        o->clientId = clientId;
        clientId = 0xFFFFFFFFu;
    }
    else {
        std::swap(gatewayPeer->vpeers[clientId], gatewayPeer->vpeers[o->clientId]);
        std::swap(clientId, o->clientId);
    }
}
