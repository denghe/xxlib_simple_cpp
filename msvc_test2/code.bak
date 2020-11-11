#include <iostream>
#include <thread>
#include "xx_chrono.h"
#include "xx_asio.h"
#include "xx_coro.h"
#include "xx_string.h"
#include "xx_lua.h"

CoAsync Delay(double const& secs) {
	auto timeoutSecs = xx::NowSteadyEpochSeconds() + secs;
	do {
		CoYield;
	} while (xx::NowSteadyEpochSeconds() > timeoutSecs);
}

struct Foo {
	~Foo() {
		std::cout << "~";
	}
};

int main() {
	auto f = [] {};
	std::tuple<int> t;
	using F = std::decay_t<decltype(f)>;
	using U = std::decay_t<decltype(t)>;
	std::cout << xx::TypeName_v<F> << std::endl;
	std::cout << xx::IsTuple_v<F> << std::endl;
	std::cout << xx::IsLambda_v<F> << std::endl;
	
	xx::Lua::State L;
	xx::Lua::SetGlobal(L, "NowSecs", [] {});

	return 0;

	xx::Asio::Client client(32768, 10);
	xx::Cors cors;
	client.onFrameUpdate = [&] {
		return (int)cors.Update();
	};
	auto co = [&]()->CoAsync {
		// 初始化拨号 ip:port  // todo: 域名解析, 包协议版本比对
		client.AddDialIP("10.0.0.13", { 12333 });

		// 无脑重置状态
	LabDial:
		CoYield;
		client.Stop();
		client.peer.reset();

		// 等 1 秒
		CoAwait(Delay(1));

		// 开始拨号( 5秒超时 ) 
		std::cout << "dial..." << std::endl;
		client.Dial(5);

		// 等拨号器变得不忙
		while (client.Busy()) {
			CoYield;
		}

		// 如果 peer 为空: 表示没有连上, 或者刚连上就被掐线. 重新拨号
		if (!client.peer) {
			std::cout << "dial timeout or peer disconnected." << std::endl;
			goto LabDial;
		}
		std::cout << "connected." << std::endl;

		// for easy use
		auto&& peer = client.peer;
		auto&& recvs = peer->recvs;

		// 设置 x 秒后自动 Close
		peer->SetTimeout(10);

		// 不断发点啥 并判断是否断线. 需要符合 4 字节长度包头格式
		while (true) {
			// 随便发点啥
			peer->Send("\1\0\0\0\1", 5);

			// 如果有收到包，就开始处理
			while (!recvs.empty()) {
				// 定位到最前面一条
				auto&& pkg = recvs.front();

				// todo: logic here
				xx::CoutN(*pkg);

				// 断线判断( 有可能上面的逻辑代码导致 )
				if (peer->closed) break;

				// 续命
				peer->SetTimeout(10);

				// 弹出最前面一条
				recvs.pop_front();
			}

			// 如果断线, 重新拨号
			if (peer->closed) {
				std::cout << "peer disconnected. reason = " << peer->closed << " desc = " << peer->closedDesc << std::endl;
				goto LabDial;
			}
			CoYield;
		}
	};
	cors.Add(co());
	// 简单模拟 60 帧执行效果
	auto lastSecs = xx::NowSteadyEpochSeconds();
	while (true) {
		std::this_thread::sleep_for(std::chrono::milliseconds(16));
		auto elapsedSecs = xx::NowSteadyEpochSeconds(lastSecs);
		if (int r = client.RunOnce(elapsedSecs)) return r;
	}
	return 0;
}



//
//int main() {
//	// 网络上下文
//	asio::io_context ioc;
//	asio::ip::udp::socket us(ioc, asio::ip::udp::endpoint(asio::ip::udp::v4(), 0));
//	asio::ip::udp::endpoint rep;
//	char d[2048];
//	asio::error_code error;
//	auto&& tar = asio::ip::udp::endpoint(asio::ip::address::from_string("192.168.1.74"), 12333);
//
//	// 帧循环上下文. 参数：时间轮长度，timer每秒处理次数
//	xx::Looper::Context ctx(8192, 10);
//
//	// 设置帧事件
//	onFrameUpdate = [&] {
//		// 先收上一帧到现在的包
//		ioc.poll();
//
//		// 网络帧逻辑处理
//		// 每帧发个包
//		us.async_receive_from(asio::buffer(d), rep, [&](const asio::error_code& e, size_t recvLen) {
//			std::cout << "frameNumber = " << frameNumber << ", e = " << e << ", recvLen = " << recvLen << std::endl;
//		});
//		us.send_to(asio::buffer("asdf"), tar);
//
//		// 看看有没有什么要立刻发出的
//		ioc.poll();
//		return 0;
//	};
//
//	// 简单模拟 client 帧延迟
//	auto lastSecs = xx::NowSteadyEpochSeconds();
//	while (true) {
//		std::this_thread::sleep_for(std::chrono::milliseconds(16));
//		secondsPool += xx::NowSteadyEpochSeconds(lastSecs);
//
//		// 试触发 FrameUpdate
//		if (int r = RunOnce()) return r;
//	}
//	return 0;
//}
