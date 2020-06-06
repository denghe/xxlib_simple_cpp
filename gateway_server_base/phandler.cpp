#include "phandler.h"
#include "peer.h"

PHandler::PHandler(Peer& peer)
    : peer(peer) {
}

Server &PHandler::GetServer() {
    return peer.GetServer();
}

void PHandler::OnReceivePackage(char *const &buf, size_t const &len) {

}

void PHandler::OnReceiveCommand(char *const &buf, size_t const &len) {

}

void PHandler::OnDisconnect(int const &reason) {

}
