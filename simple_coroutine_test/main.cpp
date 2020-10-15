#include "xx_coro.h"
#include <iostream>
#include <xx_chrono.h>

int64_t count = 0;

CoRtv Delay(int n) {
    while (n--) {
        CoYield;
        ++count;
    }
}

int main() {
    xx::Cors cs;
    for (int i = 0; i < 10000; ++i) {
        cs.Add(Delay(10000));
    }
    auto secs = xx::NowSteadyEpochSeconds();
    while (cs.Update()) {}
    std::cout << xx::NowSteadyEpochSeconds() - secs << ", " << count << std::endl;
    return 0;
}


//CoRtv Delay(int n) {
//    while(n--) CoYield;
//}
//
//CoRtv Test(int id) {
//    CoAwait(Delay(1));
//    std::cout << id << std::endl;
//    CoAwait(Delay(2));
//    std::cout << id << std::endl;
//}
//
//int main() {
//    xx::Cors cs;
//    cs.Add(Test(1));
//    cs.Add(Test(2));
//    while (!cs.Empty()) {
//        cs.Update();
//        std::cout << "Update()" << std::endl;
//    }
//    return 0;
//}

//
//CoRtv SendRequest(int peer, int pkg, int& rtv) {
//    // 这里假设 收到 Response 会放到 peer 下某字典
//    // rtv = null;   填充超时默认值
//    // int serial = ++autoInc
//    // peer->Send( serial, pkg )
//    // int timeoutTicks = 10;
//    // while(timeoutTicks--) {
//    //    CoYield
//    //    if ( peer->TryGetResponse(serial, rtv) ) break;
//    // }
//}
//
//
//CoRtv CallSendRequest(int& rtv) {
//    // 模拟 peer , pkg
//    CoAwait(SendRequest( 1, 2, rtv));
//    if (!rtv) {
//        // 超时
//        CoReturn;
//    }
//    // todo
//}
//
////CoRtv WaitAll() {
////    std::pair<int, int> r;
////    int a = cs.Add(CallSendRequest(r.first));
////    int b = cs.Add(CallSendRequest(r.second));
////    while(true) {
////        CoYield;
////        if( !cs.Find(a) && !cs.Find(b))
////            // .....
////        }
////    }
////    // r.xxxxxx
////}
