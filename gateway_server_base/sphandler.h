#pragma once
#include <cstdint>
#include "phandler.h"

// for server peer
struct SPHandler : PHandler {
    // 网关编号. 类构造后填充
    uint32_t serverId = -1;

    // 继承构造函数
    using PHandler::PHandler;

    // 收到正常包
    void OnReceivePackage(char *const &buf, size_t const &len) override;

    // 收到内部指令
    void OnReceiveCommand(char *const &buf, size_t const &len) override;

    // 断开
    void OnDisconnect(int const &reason) override;
};
