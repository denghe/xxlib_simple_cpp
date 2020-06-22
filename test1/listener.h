#pragma once
#include "xx_uv.h"
#include "peer.h"
struct Listener : xx::Uv::TcpListener<Peer> {
    using xx::Uv::TcpListener<Peer>::TcpListener;
    void Accept(std::shared_ptr<Peer> peer) override;
};
