/*
此乃 kcp(udp) 协议无关 纯转发 无状态 网关
协议说明参看 tcp 版
仅有的代码区别在于:
    从 peer.cpp,h 复制出 kpeer.cpp,h ( 小改, KPeer 继承自 EP::KcpPeer )
    CPeer 继承自 KPeer
    Listener 继承自 EP::KcpListener<CPeer>
*/

#include "xx_signal.h"
#include "config.h"
#include "server.h"
#include "xx_logger.h"

int main() {
	// 禁掉 SIGPIPE 信号避免因为连接关闭出错
	xx::IgnoreSignal();

	// 默认日志不输出到控制台
	__xxLogger.cfg.outputConsole = false;

    // 加载配置
    ajson::load_from_file(::config, "config.json");

    // 显示配置内容
    std::cout << ::config << std::endl;

	// 创建类实例
	auto&& s = xx::Make<Server>();

	// kcp 需要更高帧率运行以提供及时的响应
	s->SetFrameRate(100);

	// 开始运行
	return s->Run();
}
