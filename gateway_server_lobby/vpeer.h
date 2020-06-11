#pragma once

#include "server.h"

struct GPeer;
using GPeer_r = EP::Ref<GPeer>;

struct VPeerCB;
using VPeerCB_r = EP::Ref<VPeerCB>;

// 带超时的回调
struct VPeerCB : EP::ItemTimeout {
    // 回调函数本体
    std::function<void(char const *const &buf, size_t const &len)> func;

    // 执行 func(0,0) 后自杀
    void OnTimeout() override;
};

// 虚拟 peer
struct VPeer : EP::Item {
    // 指向 gateway peer
    GPeer_r gatewayPeer;

    // 存放位于 gateway 的 client id
    uint32_t clientId = 0xFFFFFFFFu;

    // 循环自增用于生成 序列号
    int autoIncSerial = 0;

    // 所有 带超时的回调. key: serial
    std::unordered_map<int, VPeerCB_r> callbacks;

    // 需要传入 gateway peer & client id 来初始化
    VPeer(GPeer_r const &gatewayPeer, uint32_t const &clientId);

    // 发推送
    int SendPush(char const *const &buf, size_t const &len) const;

    // 发回应
    int SendResponse(int const &serial, char const *const &buf, size_t const &len) const;

    // 发请求（收到相应回应时会触发 cb 执行。超时或断开也会触发，buf == nullptr）
    int SendRequest(char const *const &buf, size_t const &len,
                    std::function<void(char const *const &buf, size_t const &len)> &&cb, double const &timeoutSeconds);


    // 掐线 OnDisconnect(reason) + Dispose() + callbacks, 同时下发 closee( 如果物理 peer 还在的话 )
    void Disconnect(int const &reason);


    // 收到数据( 进一步解析 serial 并转发到下面几个函数 )
    void OnReceive(char const *const &buf, size_t const &len);

    // 收到回应( 自动调用 发送请求时设置的回调函数 )
    void OnReceiveResponse(uint32_t const &serial, char const *const &buf, size_t const &len);

    // 收到推送( serial == 0 ), 需要自拟业务逻辑
    virtual void OnReceivePush(char const *const &buf, size_t const &len);

    // 收到请求( serial 收到时为负数, 但传递到这里时已反转为正数 ), 需要自拟业务逻辑
    virtual void OnReceiveRequest(uint32_t const &serial, char const *const &buf, size_t const &len);

    // 断开事件
    virtual void OnDisconnect(int const &reason);

};
