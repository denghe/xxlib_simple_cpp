#include "sphandler.h"
#include "peer.h"
#include "server.h"

void SPHandler::OnReceivePackage(char *const &buf, size_t const &len) {

}

void SPHandler::OnReceiveCommand(char *const &buf, size_t const &len) {

}

void SPHandler::OnDisconnect(int const &reason) {
    // 从容器移除
    GetServer().sps.erase(id);
}
