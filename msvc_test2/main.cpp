#include <iostream>
#include <string>
#include <memory>
#include <thread>
#include <asio.hpp>
#include "xx_looper.h"
#include "xx_chrono.h"
#include "xx_asio.h"

#define COR_BEGIN    switch (lineNumber) { case 0:
#define COR_YIELD    return __LINE__; case __LINE__:;
#define COR_EXIT     return 0;
#define COR_END      } return 0;

// 预声明
struct CoroContext;

// 协程基类
struct Coro {
	// 公共上下文
	CoroContext& ctx;

	// 必须传入这个
	Coro(CoroContext& ctx) : ctx(ctx) {}
	Coro(Coro const&) = delete;
	Coro& operator=(Coro const&) = delete;
	virtual ~Coro() = default;

	// 行号
	int lineNumber = 0;

	// 执行体
	virtual int operator()() = 0;
};

// 公共上下文
struct CoroContext {
	xx::Asio::Client client;
	std::vector<std::unique_ptr<Coro>> coros;
	double lastSecs = xx::NowSteadyEpochSeconds();

	CoroContext()
		: client(32768, 60) {
		client.onFrameUpdate = [&] {
			// 倒扫以方便交换删除
			for (auto&& i = coros.size() - 1; i != (size_t)-1; --i) {
				auto&& coro = coros[i].get();
				coro->lineNumber = (*coro)();
				// 如果协程已执行完毕 就交换删除
				if (!coro->lineNumber) {
					if (i < coros.size() - 1) {
						std::swap(coros[coros.size() - 1], coros[i]);
					}
					coros.pop_back();
				}
			}
			// 如果一个不剩，通知程序退出
			return coros.empty() ? 1 : 0;
		};
	}

	int RunOnce() {
		return client.RunOnce(xx::NowSteadyEpochSeconds(lastSecs));
	}

	CoroContext(CoroContext const&) = delete;
	CoroContext& operator=(CoroContext const&) = delete;
	virtual ~CoroContext() = default;
};

struct Coro1 : Coro {
	using Coro::Coro;

	int operator()() override {
		COR_BEGIN;
		// 初始化拨号 ip:port
		ctx.client.AddDialIP("192.168.1.74", { 12333 });

		// 无脑重置状态
	LabDial:;
		COR_YIELD;
		ctx.client.Stop();
		ctx.client.peer.reset();

		// 等 1 秒
		nowSecs = xx::NowSteadyEpochSeconds();
		while (xx::NowSteadyEpochSeconds() - nowSecs < 1) {
			COR_YIELD;
		}

		std::cout << "dial..." << std::endl;
		// 开始拨号( 5秒超时 ) // todo: 域名解析, 包协议版本比对
		ctx.client.Dial(5);

		// 等拨号器变得不忙
		while (ctx.client.Busy()) {
			COR_YIELD
		}

		// 如果 peer 为空: 表示没有连上, 或者刚连上就被掐线. 重新拨号
		if (!ctx.client.peer) {
			std::cout << "dial timeout or peer disconnected." << std::endl;
			goto LabDial;
		}

		// 不断发点啥. 需要符合 4 字节长度包头格式
		while (true) {
			COR_YIELD;
			ctx.client.peer->Send("\1\0\0\0\1", 5);
		}

		COR_END;
	}

	// 存当前时间 for 定时
	double nowSecs = 0;

	// 通常用于存放返回值
	int r = 0;
};

int main() {
	CoroContext ctx;

	// 创建一个协程
	ctx.coros.emplace_back(xx::MakeU<Coro1>(ctx));

	// 简单模拟 60 帧执行效果
	auto lastSecs = xx::NowSteadyEpochSeconds();
	while (true) {
		std::this_thread::sleep_for(std::chrono::milliseconds(16));
		if (int r = ctx.RunOnce()) return r;
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
//	ctx.onFrameUpdate = [&] {
//		// 先收上一帧到现在的包
//		ioc.poll();
//
//		// 网络帧逻辑处理
//		// 每帧发个包
//		us.async_receive_from(asio::buffer(d), rep, [&](const asio::error_code& e, size_t recvLen) {
//			std::cout << "frameNumber = " << ctx.frameNumber << ", e = " << e << ", recvLen = " << recvLen << std::endl;
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
//		ctx.secondsPool += xx::NowSteadyEpochSeconds(lastSecs);
//
//		// 试触发 FrameUpdate
//		if (int r = ctx.RunOnce()) return r;
//	}
//	return 0;
//}
