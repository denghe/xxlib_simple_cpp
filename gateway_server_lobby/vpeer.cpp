#include "vpeer.h"
#include "gpeer.h"

int VPeer::SendPush(char const *const &buf, size_t const &len) const {
    return SendResponse(0, buf, len);
}

int VPeer::SendResponse(int32_t const &serial, char const *const &buf, size_t const &len) const {
    xx::Data d(len + 13);
    d.len = sizeof(uint32_t);
    d.WriteFixed(clientId);
    d.WriteVarIntger(serial);
    d.WriteBuf(buf, len);
    *(uint32_t *) d.buf = (uint32_t) (d.len - sizeof(uint32_t));
    return gatewayPeer->Send(std::move(d));
}

int VPeer::SendRequest(char const *const &buf, size_t const &len,
                       std::function<void(char const *const &buf, size_t const &len)> &&cb,
                       double const &timeoutSeconds) {
    auto&& vpcb = xx::MakeU<VPeerCB>();
    vpcb->func = std::move(cb);
    vpcb->SetTimeoutSeconds(timeoutSeconds);
    auto&& p = ep->AddItem(std::move(vpcb), -1);
    autoIncSerial = (autoIncSerial + 1) & 0x7FFFFFFF;            // uint circle use
    callbacks[autoIncSerial] = p;
    return SendResponse(-autoIncSerial, buf, len);
}

void VPeer::OnReceiveResponse(uint32_t const &serial, char const *const &buf, size_t const &len) {
    auto &&iter = callbacks.find(serial);
    if (iter == callbacks.end()) return;
    auto&& cb = iter->second.Lock();
    callbacks.erase(iter);
    if(cb) {
        cb->func(buf, len);
    }
}

void VPeer::OnDisconnect() {
    for(auto&& iter : callbacks) {
        if(iter.second) {
            iter.second->OnTimeout();
        }
    }
}

void VPeerCB::OnTimeout() {
    func(nullptr, 0);
    Dispose();
}
