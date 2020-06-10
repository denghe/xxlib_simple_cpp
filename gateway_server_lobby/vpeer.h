#pragma once
#include "server.h"

struct GPeer;
using GPeer_r = EP::Ref<GPeer>;

struct VPeerCB;
using VPeerCB_r = EP::Ref<VPeerCB>;

// 虚拟 peer
struct VPeer : EP::ItemTimeout {
    // 指向 gateway peer
    GPeer_r gatewayPeer;

    // 存放位于 gateway 的 client id
    uint32_t clientId = 0xFFFFFFFFu;

    // 循环自增用于生成 序列号
    int autoIncSerial = 0;

    // 所有 带超时的回调. key: serial
    std::unordered_map<int, VPeerCB_r> callbacks;

    // 收包事件( gateway peer 投递过来的, 已经切割过了. 可直接根据具体协议解包 )
    virtual void OnReceivePush(char const* const& buf, size_t const& len) = 0;
    virtual void OnReceiveRequest(uint32_t const& serial, char const* const& buf, size_t const& len) = 0;
    void OnReceiveResponse(uint32_t const& serial, char const* const& buf, size_t const& len);
    void OnDisconnect();

    int SendPush(char const* const& buf, size_t const& len) const;
    int SendResponse(int const& serial, char const* const& buf, size_t const& len) const;
    int SendRequest(char const* const& buf, size_t const& len
            , std::function<void(char const* const& buf, size_t const& len)>&& cb
            , double const &timeoutSeconds);
};

// 带超时的回调
struct VPeerCB : EP::ItemTimeout {
    // 回调函数本体
    std::function<void(char const* const& buf, size_t const& len)> func;

    // 执行 func(0,0) 后自杀
    void OnTimeout() override;
};
