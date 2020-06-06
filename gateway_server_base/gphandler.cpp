#include "gphandler.h"
#include "peer.h"
#include "server.h"

void GPHandler::OnReceivePackage(char *const &buf, size_t const &len) {

}

void GPHandler::OnReceiveCommand(char *const &buf, size_t const &len) {

}

void GPHandler::OnDisconnect(int const &reason) {
    // 从容器移除
    GetServer().gps.erase(id);
}
