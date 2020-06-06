#pragma once
#include <cstddef>  // for size_t
#include <cstdint>

struct Peer;
struct Server;

// Peer Handler 基础接口
struct PHandler {
    // 引用到 peer
    Peer& peer;

    // 复用编号( 自增 | 网关 | 服务 ). 类构造后填充
    uint32_t id = -1;

    // 初始化 peer 引用
    explicit PHandler(Peer& peer);
    PHandler(PHandler const&) = delete;
    PHandler& operator=(PHandler const&) = delete;
    virtual ~PHandler() = default;

    // 等同于 peer.GetServer()
    Server &GetServer();

    // 等同于 peer.Dispose()
    void Dispose();

    // 收到正常包
    virtual void OnReceivePackage(char *const &buf, size_t const &len) = 0;

    // 收到内部指令
    virtual void OnReceiveCommand(char *const &buf, size_t const &len) = 0;

    // 断开( 析构不会触发 ): 将 peer 从相应容器移除
    virtual void OnDisconnect(int const &reason) = 0;
};
