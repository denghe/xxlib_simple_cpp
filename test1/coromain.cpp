#include "coromain.h"
#include "client.h"
#include <iostream>

int CoroMain::Update() {
    COR_BEGIN
            // 初始化 域名解析器
            xx::MakeTo(resolver, c.shared_from_this());
        LabResolve:
        COR_YIELD
            // 停掉域名解析
            resolver->Stop();

            // 等 1 秒( 必须的. 除了防止低延迟疯狂拨号以外, 还可以令别的协程有机会处理断线逻辑 )
            nowSecs = xx::NowSteadyEpochSeconds();
            while (xx::NowSteadyEpochSeconds() - nowSecs < 1) {
                COR_YIELD
            }

            std::cout << "Resolve..." << std::endl;
            if (int r = resolver->Resolve("www.baidu.com")) {
                std::cout << "Resolve failed. r = " << r << std::endl;
                goto LabResolve;
            }

            // 等域名解析器变得不忙
            std::cout << "wait busy..." << std::endl;
            while (resolver->Busy()) {
                COR_YIELD
            }

            // 如果 ips 为空: 表示没有在规定时间内解析出来
            if (resolver->ips.empty()) {
                std::cout << "Resolve timeout" << std::endl;
                goto LabResolve;
            }
            else {
                std::cout << "Resolve success!" << std::endl;
                for (auto &&ip : resolver->ips) {
                    std::cout << ip << std::endl;
                }
            }
    COR_END
}
