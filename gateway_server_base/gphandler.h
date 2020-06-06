#pragma once
#include <cstdint>
#include "phandler.h"

// for gateway peer
struct GPHandler : PHandler {
    // 继承构造函数
    using PHandler::PHandler;

    // 收到正常包
    void OnReceivePackage(char *const &buf, size_t const &len) override;

    // 收到内部指令
    void OnReceiveCommand(char *const &buf, size_t const &len) override;

    // 断开
    void OnDisconnect(int const &reason) override;
};
