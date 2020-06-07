#pragma once
#include <cstddef>  // for size_t
#include <cstdint>

struct Peer;
struct Server;

// Peer Handler 基础接口. 挂接在 peer 下面。 peer Dispose 也将导致本接口析构
struct PHandler {
    // 引用到所在 peer
    Peer& peer;

    // 复用编号( 自增 | 网关 | 服务 )
    uint32_t id = -1;

    // 初始化 peer 引用
    explicit PHandler(Peer& peer, uint32_t const& id);
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
