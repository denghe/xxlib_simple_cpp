#include "peer.h"
void Peer::Receive() {
    // echo
    Send(recv.buf, recv.len);
    recv.Clear();
    SetTimeoutSeconds(30);
}
