#include <iostream>
#include <string>
#include <memory>
#include <thread>
#include <asio.hpp>
#include "xx_looper.h"
#include "xx_chrono.h"


int main() {
	// ����������
	asio::io_context ioc;
	asio::ip::udp::socket us(ioc, asio::ip::udp::endpoint(asio::ip::udp::v4(), 0));
	asio::ip::udp::endpoint rep;
	char d[2048];
	asio::error_code error;
	auto&& tar = asio::ip::udp::endpoint(asio::ip::address::from_string("192.168.1.235"), 12345);

	// ֡ѭ��������. ������ʱ���ֳ��ȣ�timerÿ�봦�����
	xx::Looper::Context ctx(8192, 10);

	// ����֡�¼�
	ctx.onFrameUpdate = [&] {
		// ������һ֡�����ڵİ�
		ioc.poll();

		// ����֡�߼�����
		// ÿ֡������
		us.async_receive_from(asio::buffer(d), rep, [&](const asio::error_code& e, size_t recvLen) {
			std::cout << "frameNumber = " << ctx.frameNumber << ", e = " << e << ", recvLen = " << recvLen << std::endl;
		});
		us.send_to(asio::buffer("asdf"), tar);

		// ������û��ʲôҪ���̷�����
		ioc.poll();
		return 0;
	};

	// ��ģ�� client ֡�ӳ�
	auto lastSecs = xx::NowSteadyEpochSeconds();
	while (true) {
		std::this_thread::sleep_for(std::chrono::milliseconds(16));
		ctx.secondsPool += xx::NowSteadyEpochSeconds(lastSecs);

		// �Դ��� FrameUpdate
		if (int r = ctx.RunOnce()) return r;
	}
	return 0;
}
