#pragma once

#include "server.h"

struct GPeer;
struct VPeer;

// 带超时的回调
struct VPeerCB : EP::Timer {
    // 所在 vpeer( 移除的时候要用到 )
    std::shared_ptr<VPeer> vpeer;

    // 序号( 移除的时候要用到 )
    int serial = 0;

    // 回调函数本体
    std::function<void(char const *const &buf, size_t const &len)> func;

    // 继承构造函数
    VPeerCB(std::shared_ptr<VPeer> const& vpeer, int const& serial
            , std::function<void(char const *const &buf, size_t const &len)>&& cbfunc, double const &timeoutSeconds);

    // 执行 func(0,0) 后 从容器移除, 并延迟 Unhold
    void Timeout() override;
};

// 虚拟 peer
struct VPeer : EP::Item {
    // 指向 gateway peer
    std::shared_ptr<GPeer> gatewayPeer;

    // 存放位于 gateway 的 client id
    uint32_t clientId = 0xFFFFFFFFu;

    // 循环自增用于生成 序列号
    int autoIncSerial = 0;

    // 所有 带超时的回调. key: serial
    std::unordered_map<int, std::shared_ptr<VPeerCB>> callbacks;

    // 需要传入 gateway peer & client id 来初始化
    VPeer(std::shared_ptr<GPeer> const &gatewayPeer, uint32_t const &clientId);

    // 发推送
    int SendPush(char const *const &buf, size_t const &len) const;

    // 发回应
    int SendResponse(int const &serial, char const *const &buf, size_t const &len) const;

    // 发请求（收到相应回应时会触发 cb 执行。超时或断开也会触发，buf == nullptr）
    int SendRequest(char const *const &buf, size_t const &len,
                    std::function<void(char const *const &buf, size_t const &len)> &&cb, double const &timeoutSeconds);


    // 掐线 OnDisconnect(reason) + callbacks + delay unhold, 同时下发 close( 如果物理 peer Alive() 的话 )
    bool Close(int const &reason) override;

    // 收到数据( 进一步解析 serial 并转发到下面几个函数 )
    void Receive(char const *const &buf, size_t const &len);

    // 收到回应( 自动调用 发送请求时设置的回调函数 )
    void ReceiveResponse(uint32_t const &serial, char const *const &buf, size_t const &len);

    // 收到推送( serial == 0 ), 需要自拟业务逻辑
    virtual void ReceivePush(char const *const &buf, size_t const &len);

    // 收到请求( serial 收到时为负数, 但传递到这里时已反转为正数 ), 需要自拟业务逻辑
    virtual void ReceiveRequest(uint32_t const &serial, char const *const &buf, size_t const &len);

    // 断开事件
    virtual void OnDisconnect(int const &reason);

    // 和另一个 vpeer 交换 clientId & mappings
    void SwapClientId(std::shared_ptr<VPeer> const& o);
};
