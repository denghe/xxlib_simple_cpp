#include "server.h"
#include "lpeer.h"
#include "xx_datareader.h"

void LPeer::OnReceivePackage(char *const &buf, size_t const &len) {
    // todo
}

void LPeer::OnReceiveFirstPackage(char *const &buf, size_t const &len) {
    assert(false);
}

void LPeer::OnDisconnect(int const &reason) {
    // todo
}
