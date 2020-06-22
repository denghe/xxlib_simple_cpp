#pragma once
#include "xx_uv.h"
struct Peer : xx::Uv::TcpPeer {
    using xx::Uv::TcpPeer::TcpPeer;
    void Receive() override;
};
