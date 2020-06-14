/*
模拟了一个 game 服务. 被 gateways 连. 连到 lobby 服务.
设计要点:
    一个 listener, 监听一个端口
    accept 之后, 先产生 "匿名peer". 此时, 只接受 "gatewayId" 指令
    在解析指令之后, 根据 接入者类型, 创建相应的 事件处理类
    一个 dialer, 自动连向 lobby. 连上之后, 发注册包


下文中 指令 使用函数长相来描述 ( 以函数名 string 打头, 后跟参数 )

gateway 过来的 指令列表：

    gatewayId(uint32_t gatewayId)
        首包, 注册网关. 如果收到该包时发现 id 已存在，则忽略并断线( 不顶下线 )

    accept(uint32_t clientId, string ip)
        客户端接入. 如果允许, 应向该 client 发送 open 指令 并创建相应的虚拟 peer 处理逻辑. 否则 发送 kick

    disconnect(uint32_t clientId)
        client 断线通知, 应立刻杀掉相应的 虚拟 peer

gateway 除了发送指令以外, 还转发 client 数据过来. 直接路由给 相应的 虚拟 peer
gateway 如果断开, 则断开所有与之关联的 虚拟 peer

发向 gateway 的 指令列表：

    open(uint32_t clientId)
        通知 gateway 打开指定 client 与当前服务的通信信道( gateway: sid 纳入 白名单, 下发 open )

    close(uint32_t clientId)
        通知 gateway 关闭指定 client 与当前服务的通信信道( gateway: sid 从 白名单 移除, 下发 close )

    kick(uint32_t clientId, double delaySeconds)
        通知 gateway 踢掉指定 client ( gateway: 踢人 )



发向 lobby 的 指令列表：

    serverId(uint32_t serverId)
        首包, 注册某服务( 具体是啥可以进一步 switch case ). 如果收到该包时发现 id 已存在，则忽略并断线( 不顶下线 )


*/

#include "xx_signal.h"
#include "config.h"
#include "server.h"

int main() {
    // 禁掉 SIGPIPE 信号避免因为连接关闭出错
    xx::IgnoreSignal();

    // 加载配置
    ajson::load_from_file(::config, "config.json");

    // 显示配置内容
    std::cout << ::config << std::endl;

    // 创建类实例
    auto&& s = xx::Make<Server>();

    // 开始运行
    return s->Run(10);
}
