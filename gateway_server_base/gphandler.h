#pragma once
#include <cstdint>
#include "phandler.h"

// for gateway peer
struct GPHandler : PHandler {
    // 放入容器
    GPHandler(Peer& peer, uint32_t const& id);

    // 从容器移除
    ~GPHandler() override;

    // 收到正常包
    void OnReceivePackage(char *const &buf, size_t const &len) override;

    // 收到内部指令
    void OnReceiveCommand(char *const &buf, size_t const &len) override;
};
