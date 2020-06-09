/*
大体流程:
    1. client dial to gateway
    2. client wait open( serverId )
    3. client send request to serverId: pkg
    4. client wait callback
    5. .......

客户端会收到两类数据：
    内部指令
        LEN(uint) + 0xFFFFFFFF(uint) + "open"(string) + serverId(uint)
        LEN(uint) + 0xFFFFFFFF(uint) + "close"(string) + serverId(uint)
    普通包
        LEN(uint) + serverId(uint) + data( 格式未知 )
*/

#include "xx_signal.h"
#include "client.h"

int main() {
    // 禁掉 SIGPIPE 信号避免因为连接关闭出错
    xx::IgnoreSignal();

    // 创建类实例
    Client c;

    // 开始运行
    return c.Run();
}
