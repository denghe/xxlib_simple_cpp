#include "xx_signal.h"
#include "xx_uv.h"

int main() {
    // 禁掉 SIGPIPE 信号避免因为连接关闭出错
    xx::IgnoreSignal();
	
	return 0;
}
