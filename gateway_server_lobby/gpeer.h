#pragma once

#include "peer.h"

struct VPeer;

// gateway 连进来产生的 peer
struct GPeer : Peer {
    // 所有的属于这个网关接入的虚拟peer集合. key: clientId
    std::unordered_map<uint32_t, std::shared_ptr<VPeer>> vpeers;

    // 继承构造函数
    using Peer::Peer;

    // 从相应容器中移除 + 遍历调用 vpeers 的 Close
    bool Close(int const &reason) override;

    // 收到正常包
    void ReceivePackage(char *const &buf, size_t const &len) override;

    // 收到首包( 拿到 gatewayId, 放入相应容器 )
    void ReceiveFirstPackage(char *const &buf, size_t const &len) override;

    // 构造内部指令包. LEN + ADDR + cmd string + args...
    template<typename...Args>
    void SendTo(uint32_t const &id, Args const &... cmdAndArgs) {
        xx::Data d(32);
        d.len = sizeof(uint32_t);
        d.WriteFixed(id);
        xx::Write(d, cmdAndArgs...);
        *(uint32_t *) d.buf = (uint32_t) (d.len - sizeof(uint32_t));
        this->Send(std::move(d));
    }
};
