#include "phandler.h"
#include "peer.h"

PHandler::PHandler(Peer& peer, uint32_t const& id)
    : peer(peer)
    , id(id) {
}
