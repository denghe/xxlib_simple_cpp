#pragma once
#include <cstdint>
#include "phandler.h"

struct SPeer;

// for server peer
struct SPHandler : PHandler<SPeer> {
    // 放入容器
    SPHandler(SPeer& peer, uint32_t const& id);

    // 从容器移除
    ~SPHandler() override;

    // 收到正常包
    void OnReceivePackage(char *const &buf, size_t const &len) override;

    // 收到内部指令
    void OnReceiveCommand(char *const &buf, size_t const &len) override;
};
