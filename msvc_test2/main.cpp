#include <iostream>
#include <string>
#include <memory>
#include <thread>
#include <asio.hpp>
#include "xx_looper.h"
#include "xx_chrono.h"


int main() {
	xx::Looper::Context ctx(8192, 10);	// 8192轮长 / 10帧 = 最大超时时长 819 秒
	

	auto lastSecs = xx::NowSteadyEpochSeconds();
	while (true) {
		// 模拟客户端帧延迟
		std::this_thread::sleep_for(std::chrono::milliseconds(100));
		auto nowSecs = xx::NowSteadyEpochSeconds();
		ctx.secondsPool += nowSecs - lastSecs;
		lastSecs = nowSecs;
		if (ctx.RunCheck()) {
			ctx.RunOnce();
		}
	}
	return 0;
}
