#pragma once
#include <cstddef>  // for size_t

struct Peer;
struct Server;

// Peer Handler 基础接口
struct PHandler {
    // 引用到 peer
    Peer& peer;

    PHandler(Peer& peer);

    // 拿到 server 上下文引用, 以方便写事件处理代码
    Server &GetServer();

    // 收到正常包
    virtual void OnReceivePackage(char *const &buf, size_t const &len);

    // 收到内部指令
    virtual void OnReceiveCommand(char *const &buf, size_t const &len);

    // 断开时 清除所有 client peer 中的 相关 open id. 列表被清空则踢掉
    virtual void OnDisconnect(int const &reason);
};
