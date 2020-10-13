#include "xx_signal.h"
#include "server.h"
#include "generator.h"

int main() {
	// 禁掉 SIGPIPE 信号避免因为连接关闭出错
	xx::IgnoreSignal();

	// 创建类实例. 轮长 = 最大超时秒数 * 帧数 的 2^n 对齐. kcp 需要每秒 100 帧
	auto&& s = xx::Make<Server>(32768, 100);

	// 开始运行
	return s->Run();
}
