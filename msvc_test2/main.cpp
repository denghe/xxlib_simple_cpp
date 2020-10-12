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

// Ԥ����
struct CoroContext;

// Э�̻���
struct Coro {
	// ����������
	CoroContext& ctx;

	// ���봫�����
	Coro(CoroContext& ctx) : ctx(ctx) {}
	Coro(Coro const&) = delete;
	Coro& operator=(Coro const&) = delete;
	virtual ~Coro() = default;

	// �к�
	int lineNumber = 0;

	// ִ����
	virtual int operator()() = 0;
};

// ����������
struct CoroContext {
	xx::Asio::Client client;
	std::vector<std::unique_ptr<Coro>> coros;
	double lastSecs = xx::NowSteadyEpochSeconds();

	CoroContext()
		: client(32768, 60) {
		client.onFrameUpdate = [&] {
			// ��ɨ�Է��㽻��ɾ��
			for (auto&& i = coros.size() - 1; i != (size_t)-1; --i) {
				auto&& coro = coros[i].get();
				coro->lineNumber = (*coro)();
				// ���Э����ִ����� �ͽ���ɾ��
				if (!coro->lineNumber) {
					if (i < coros.size() - 1) {
						std::swap(coros[coros.size() - 1], coros[i]);
					}
					coros.pop_back();
				}
			}
			// ���һ����ʣ��֪ͨ�����˳�
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
		// ��ʼ������ ip:port
		ctx.client.AddDialIP("192.168.1.74", { 12333 });

		// ��������״̬
	LabDial:;
		COR_YIELD;
		ctx.client.Stop();
		ctx.client.peer.reset();

		// �� 1 ��
		nowSecs = xx::NowSteadyEpochSeconds();
		while (xx::NowSteadyEpochSeconds() - nowSecs < 1) {
			COR_YIELD;
		}

		std::cout << "dial..." << std::endl;
		// ��ʼ����( 5�볬ʱ ) // todo: ��������, ��Э��汾�ȶ�
		ctx.client.Dial(5);

		// �Ȳ�������ò�æ
		while (ctx.client.Busy()) {
			COR_YIELD
		}

		// ��� peer Ϊ��: ��ʾû������, ���߸����Ͼͱ�����. ���²���
		if (!ctx.client.peer) {
			std::cout << "dial timeout or peer disconnected." << std::endl;
			goto LabDial;
		}

		// ���Ϸ���ɶ. ��Ҫ���� 4 �ֽڳ��Ȱ�ͷ��ʽ
		while (true) {
			COR_YIELD;
			ctx.client.peer->Send("\1\0\0\0\1", 5);
		}

		COR_END;
	}

	// �浱ǰʱ�� for ��ʱ
	double nowSecs = 0;

	// ͨ�����ڴ�ŷ���ֵ
	int r = 0;
};

int main() {
	CoroContext ctx;

	// ����һ��Э��
	ctx.coros.emplace_back(xx::MakeU<Coro1>(ctx));

	// ��ģ�� 60 ִ֡��Ч��
	auto lastSecs = xx::NowSteadyEpochSeconds();
	while (true) {
		std::this_thread::sleep_for(std::chrono::milliseconds(16));
		if (int r = ctx.RunOnce()) return r;
	}
	return 0;
}



//
//int main() {
//	// ����������
//	asio::io_context ioc;
//	asio::ip::udp::socket us(ioc, asio::ip::udp::endpoint(asio::ip::udp::v4(), 0));
//	asio::ip::udp::endpoint rep;
//	char d[2048];
//	asio::error_code error;
//	auto&& tar = asio::ip::udp::endpoint(asio::ip::address::from_string("192.168.1.74"), 12333);
//
//	// ֡ѭ��������. ������ʱ���ֳ��ȣ�timerÿ�봦�����
//	xx::Looper::Context ctx(8192, 10);
//
//	// ����֡�¼�
//	ctx.onFrameUpdate = [&] {
//		// ������һ֡�����ڵİ�
//		ioc.poll();
//
//		// ����֡�߼�����
//		// ÿ֡������
//		us.async_receive_from(asio::buffer(d), rep, [&](const asio::error_code& e, size_t recvLen) {
//			std::cout << "frameNumber = " << ctx.frameNumber << ", e = " << e << ", recvLen = " << recvLen << std::endl;
//		});
//		us.send_to(asio::buffer("asdf"), tar);
//
//		// ������û��ʲôҪ���̷�����
//		ioc.poll();
//		return 0;
//	};
//
//	// ��ģ�� client ֡�ӳ�
//	auto lastSecs = xx::NowSteadyEpochSeconds();
//	while (true) {
//		std::this_thread::sleep_for(std::chrono::milliseconds(16));
//		ctx.secondsPool += xx::NowSteadyEpochSeconds(lastSecs);
//
//		// �Դ��� FrameUpdate
//		if (int r = ctx.RunOnce()) return r;
//	}
//	return 0;
//}
