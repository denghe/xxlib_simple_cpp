#include "phandler.h"
#include "peer.h"

PHandler::PHandler(Peer& peer)
    : peer(peer) {
}

Server &PHandler::GetServer() {
    return peer.GetServer();
}

void PHandler::Dispose() {
    peer.Dispose();
}
