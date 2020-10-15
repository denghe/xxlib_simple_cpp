#include "xx_signal.h"
#include "server.h"
#include "xx_coro.h"

CoRtv Delay(int n) {
    while(n--) CoYield;
}

CoRtv Test() {
    CoAwait(Delay(1));
    std::cout << "1111111111" << std::endl;
    CoAwait(Delay(2));
    std::cout << "22222222222222" << std::endl;
    CoAwait(Delay(3));
    std::cout << "333333333333333333" << std::endl;
}

CoRtv SendRequest(int peer, int pkg, int& rtv) {
    // 这里假设 收到 Response 会放到 peer 下某字典
    // rtv = null;   填充超时默认值
    // int serial = ++autoInc
    // peer->Send( serial, pkg )
    // int timeoutTicks = 10;
    // while(timeoutTicks--) {
    //    CoYield
    //    if ( peer->TryGetResponse(serial, rtv) ) break;
    // }
}

xx::Cors cs;

CoRtv CallSendRequest(int& rtv) {
    // 模拟 peer , pkg
    CoAwait(SendRequest( 1, 2, rtv));
    if (!rtv) {
        // 超时
        CoReturn;
    }
    // todo
}

//CoRtv WaitAll() {
//    std::pair<int, int> r;
//    int a = cs.Add(CallSendRequest(r.first));
//    int b = cs.Add(CallSendRequest(r.second));
//    while(true) {
//        CoYield;
//        if( !cs.Find(a) && !cs.Find(b))
//            // .....
//        }
//    }
//    // r.xxxxxx
//}

int main() {
    cs.Add(Test());
    while (!cs.Empty()) {
        cs.Update();
        std::cout << "Update()" << std::endl;
    }
    return 0;

//    GetInts2().Foreach([](auto&& i){
//        std::cout << i << std::endl;
//    });
//    return 0;

    // 禁掉 SIGPIPE 信号避免因为连接关闭出错
    xx::IgnoreSignal();

    // 创建类实例. 轮长 = 最大超时秒数 * 帧数 的 2^n 对齐. kcp 需要每秒 100 帧
    auto &&s = xx::Make<Server>(32768, 100);

    // 开始运行
    return s->Run();
}

//xx::Generator<int> GetInts() {
//    for (int i = 0; i < 5; ++i) {
//        co_yield i;
//    }
//}
//
//xx::Generator<int> GetInts2() {
//    auto g = GetInts();
//    for (int i = 0; i < 10; ++i) {
//        if (g.Next()) co_yield g.Value();
//        else g = GetInts();
//    }
//}