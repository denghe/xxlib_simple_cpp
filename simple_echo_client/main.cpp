#include "xx_signal.h"
#include "client.h"

int main() {
	// 禁掉 SIGPIPE 信号避免因为连接关闭出错
	xx::IgnoreSignal();

	// 创建类实例
    auto &&c = xx::Make<Client>();

	// 开始运行
	return c->Run(10);
}
