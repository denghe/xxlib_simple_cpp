#include "hpeer.h"

bool HPeer::Close(int const &reason, char const *const &desc) {
    xx::CoutN(addr, " Close. line number = ", reason, " file name = ", desc);
    return this->HttpPeerEx::Close(reason, desc);
}

HPeer::~HPeer() {
}

void HPeer::ReceiveHttp() {
    xx::CoutN(addr, "recv http. method = ", method, ", url = ", url, ", body = ", body);
    if (url=="/favicon.ico") {
        Send404("");
    }
    else {
        SendText("hi!");
    }
    SetTimeoutSeconds(keepAlive ? 30 : 3);
}
