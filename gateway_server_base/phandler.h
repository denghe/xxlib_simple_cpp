#pragma once
#include <cstddef>  // for size_t

struct Peer;
struct Server;

// Peer Handler 基础接口
struct PHandler {
    // 引用到 peer
    Peer& peer;

    // 初始化 peer 引用
    PHandler(Peer& peer);
    PHandler(PHandler const&) = delete;
    PHandler& operator=(PHandler const&) = delete;

    // 等同于 peer.GetServer()
    Server &GetServer();

    // 等同于 peer.Dispose()
    void Dispose();

    // 收到正常包
    virtual void OnReceivePackage(char *const &buf, size_t const &len) = 0;

    // 收到内部指令
    virtual void OnReceiveCommand(char *const &buf, size_t const &len) = 0;

    // 断开
    virtual void OnDisconnect(int const &reason) = 0;
};
