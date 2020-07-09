/*
大体流程:
    1. client dial to gateway
    2. client wait open( serverId == 0 )
    3. client send request to server: auth?
    4. client wait response( return to game serverId = xxx? )
    5. client wait open( serverId == xxx )
    6. enter game? send pkg? ...

注意：
    上面的每个步骤，理论上讲都要有自己的 超时检测

客户端会收到两类数据：
    内部指令
        LEN(uint) + 0xFFFFFFFF(uint) + "open"(string) + serverId(uint)
        LEN(uint) + 0xFFFFFFFF(uint) + "close"(string) + serverId(uint)
    普通包
        LEN(uint) + serverId(uint) + data( 格式未知 )

*/

#include "xx_signal.h"
#include "client.h"
#include "config.h"

int main() {
    // 禁掉 SIGPIPE 信号避免因为连接关闭出错
    xx::IgnoreSignal();

    // 加载配置
    ajson::load_from_file(::config, "config.json");

    // 显示配置内容
    std::cout << ::config << std::endl;

    // 创建类实例
    auto&& c = xx::Make<Client>();

    // 开始运行( 每秒 60 帧 )
    c->SetFrameRate(60);
    return c->Run();
}
