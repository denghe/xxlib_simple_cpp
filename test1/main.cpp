#include "xx_signal.h"
#include "client.h"
#include "coromain.h"
#include <iostream>

namespace UV = xx::Uv;

int main() {
    // 禁掉 SIGPIPE 信号避免因为连接关闭出错
    xx::IgnoreSignal();
    auto &&c = std::make_shared<Client>();
    std::cout << "c.use_count() = " << c.use_count() << std::endl;

    // 创建默认协程
    c->CreateCoro<CoroMain>();

    int r = c->Run(100);
    std::cout << "Run r = " << r << std::endl;

    std::cout << "c.use_count() = " << c.use_count() << std::endl;

    return 0;
}
