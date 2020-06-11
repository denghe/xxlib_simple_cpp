#include "vpeer.h"
#include "gpeer.h"

void VPeerCB::OnTimeout() {
    // 传递空参数以体现这是超时
    func(nullptr, 0);
    // 执行完立刻自杀
    Dispose();
}

VPeer::VPeer(GPeer_r const &gatewayPeer, uint32_t const &clientId)
        : gatewayPeer(gatewayPeer),
          clientId(clientId) {
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
                       std::function<void(char const *const &buf, size_t const &len)> &&cb,
                       double const &timeoutSeconds) {
    // 创建一个 带超时的回调 将 cb 封装起来
    auto &&vpcb = xx::MakeU<VPeerCB>();
    vpcb->func = std::move(cb);
    vpcb->SetTimeoutSeconds(timeoutSeconds);
    auto &&p = ep->AddItem(std::move(vpcb), -1);
    // 以序列号建立cb的映射
    autoIncSerial = (autoIncSerial + 1) & 0x7FFFFFFF;            // uint circle use
    callbacks[autoIncSerial] = p;
    // 发包并返回( 请求性质的包, 序号为负数 )
    return SendResponse(-autoIncSerial, buf, len);
}

void VPeer::Disconnect(int const &reason) {
    // 如果物理 peer 还在
    if (auto &&gp = gatewayPeer.Lock()) {
        // 通过网关向客户端发 close 指令
        gp->SendTo(0xFFFFFFFFu, "close", clientId);
        // 从容器移除
        gp->vpeers.erase(clientId);
    }
    // 触发断线回调
    OnDisconnect(reason);
    // 将回调容器移到栈上，以便 Dispose 之后继续访问
    auto cbs = std::move(callbacks);
    // 自杀
    Dispose();
    // 触发所有已存在回调（ 模拟立刻超时 ）
    for (auto &&iter : cbs) {
        if (auto &&cb = iter.second.Lock()) {
            // 回调中可自己探测 vpeer 是否已阵亡，可分辨是 一般超时（未断开） 还是 断线的伪超时
            cb->func(nullptr, 0);
            cb->Dispose();
        }
    }
}


void VPeer::OnReceive(char const *const &buf, size_t const &len) {
    // 试读出序号. 出错直接断开退出
    int serial = 0;
    xx::DataReader dr(buf, len);
    if (dr.Read(serial)) {
        Disconnect(__LINE__);
        return;
    }

    // 根据序列号的情况分性质转发
    if (serial == 0) {
        OnReceivePush(buf + dr.offset, len - dr.offset);
    } else if (serial > 0) {
        OnReceiveResponse(serial, buf + dr.offset, len - dr.offset);
    } else {
        // -serial: 将 serial 转为正数
        OnReceiveRequest(-serial, buf + dr.offset, len - dr.offset);
    }
}

void VPeer::OnReceiveResponse(uint32_t const &serial, char const *const &buf, size_t const &len) {
    // 根据序号定位到 cb
    auto &&iter = callbacks.find(serial);
    if (iter == callbacks.end()) return;
    // 先取出来并从回调容器移除( 防止 callback 中有自杀行为导致中间逻辑不方便判断 )
    auto &&cb = iter->second.Lock();
    callbacks.erase(iter);
    // 如果 cb 有效（未超时Dispose）则执行( 后面没有代码, 回调中导致自杀没有副作用 )
    if (cb) {
        cb->func(buf, len);
        cb->Dispose();
    }
}


void VPeer::OnReceivePush(char const *const &buf, size_t const &len) {
    // 模拟某协议解包
    std::string txt;
    if (xx::Read(buf, len, txt)) {
        Disconnect(__LINE__);
        return;
    }
    std::cout << "vpeer: " << clientId << " recv push: " << txt << std::endl;
    // todo
}

void VPeer::OnReceiveRequest(uint32_t const &serial, char const *const &buf, size_t const &len) {
    // 模拟某协议解包
    std::string txt;
    if (xx::Read(buf, len, txt)) {
        Disconnect(__LINE__);
        return;
    }
    std::cout << "vpeer: " << clientId << " recv request: " << txt << std::endl;

    if (txt == "auth") {
        // 模拟构造回包: 服务类型 + serverId
        xx::Data d;
        xx::Write(d, "lobby", (uint32_t)0);
        SendResponse(serial, d.buf, d.len);
        // 通知 gateway open 指定 serverId?
    } else if (txt == "info") {
        // 模拟构造回包
        xx::Data d;
        xx::Write(d, "username:xxxxx coin:123123123");
        SendResponse(serial, d.buf, d.len);
    } else {
        // 未知包?
        Disconnect(__LINE__);
        return;
    }
}

void VPeer::OnDisconnect(int const &reason) {
    std::cout << "vpeer: " << clientId << " disconnected. reason: " << reason << std::endl;
    // todo
}
