#include <iostream>
#include <string>
#include <memory>
#include <thread>
#include <asio.hpp>
#include "xx_looper.h"
#include "xx_chrono.h"


int main() {
	// 网络上下文
	asio::io_context ioc;
	asio::ip::udp::socket us(ioc, asio::ip::udp::endpoint(asio::ip::udp::v4(), 0));
	asio::ip::udp::endpoint rep;
	char d[2048];
	asio::error_code error;
	auto&& tar = asio::ip::udp::endpoint(asio::ip::address::from_string("192.168.1.235"), 12345);

	// 帧循环上下文. 参数：时间轮长度，timer每秒处理次数
	xx::Looper::Context ctx(8192, 10);

	// 设置帧事件
	ctx.onFrameUpdate = [&] {
		// 先收上一帧到现在的包
		ioc.poll();

		// 网络帧逻辑处理
		// 每帧发个包
		us.async_receive_from(asio::buffer(d), rep, [&](const asio::error_code& e, size_t recvLen) {
			std::cout << "frameNumber = " << ctx.frameNumber << ", e = " << e << ", recvLen = " << recvLen << std::endl;
		});
		us.send_to(asio::buffer("asdf"), tar);

		// 看看有没有什么要立刻发出的
		ioc.poll();
		return 0;
	};

	// 简单模拟 client 帧延迟
	auto lastSecs = xx::NowSteadyEpochSeconds();
	while (true) {
		std::this_thread::sleep_for(std::chrono::milliseconds(16));
		ctx.secondsPool += xx::NowSteadyEpochSeconds(lastSecs);

		// 试触发 FrameUpdate
		if (int r = ctx.RunOnce()) return r;
	}
	return 0;
}
