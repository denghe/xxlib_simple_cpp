#pragma once
#include <cstddef>  // for size_t
#include <cstdint>

// Peer Handler 基础接口. 挂接在 peer 下面。 peer Dispose 也将导致本接口析构
template<typename PeerType>
struct PHandler {
    // 引用到所在 peer
    PeerType& peer;

    // 复用编号( 自增 | 网关 )
    uint32_t id = -1;

    // 初始化 peer 引用
    explicit PHandler(PeerType& peer, uint32_t const& id) : peer(peer), id(id) {}
    PHandler(PHandler const&) = delete;
    PHandler& operator=(PHandler const&) = delete;
    virtual ~PHandler() = default;

    // 收到正常包
    virtual void OnReceivePackage(char *const &buf, size_t const &len) = 0;

    // 收到内部指令
    virtual void OnReceiveCommand(char *const &buf, size_t const &len) = 0;

    // 断开
    inline virtual void OnDisconnect(int const &reason) {}
};
