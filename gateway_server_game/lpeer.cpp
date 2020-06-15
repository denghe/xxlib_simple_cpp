#include "server.h"
#include "lpeer.h"
#include "xx_datareader.h"

void LPeer::ReceivePackage(char *const &buf, size_t const &len) {
    // todo
}

void LPeer::ReceiveFirstPackage(char *const &buf, size_t const &len) {
    assert(false);
    // todo
}

bool LPeer::Close(int const &reason) {
    // 防重入( 同时关闭 fd )
    if (!this->Item::Close(reason)) return false;
    // 减持
    GetServer().lobbyPeer.reset();
    // 延迟减持
    DelayUnhold();
    return true;
}
