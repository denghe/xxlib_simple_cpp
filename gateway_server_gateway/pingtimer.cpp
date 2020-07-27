#include "pingtimer.h"
#include "server.h"
#include "speer.h"

void PingTimer::Timeout() {
    auto &&server = *(Server *) &*ec;
    // 遍历当前已存在的服务器间连接容器
    for (auto &&dp : server.dps) {
        // 如果连接存在
        if (auto &&sp = dp.second.second) {
            // 如果正在等
            if (sp->waitingPingBack) {
                // 如果已经等了好些时候了( 该值可配？）
                if(server.nowMS - sp->lastSendPingMS > 5000) {
                    // 掐线
                    sp->Close(__LINE__, __FILE__);
                }
            }
            else {
                // 发起 ping
                sp->SendCommand("ping", server.nowMS);
                sp->waitingPingBack = true;
            }
        }
    }

    // timer 续命。下次在 1 秒后
    SetTimeoutSeconds(1);
}
