#include <iostream>
#include <string>
#include <memory>
#include <chrono>
#include <asio.hpp>

int main() {
	try {
		asio::io_context ioc;
		asio::ip::udp::socket us(ioc, asio::ip::udp::endpoint(asio::ip::udp::v4(), 0));
		asio::ip::udp::endpoint rep;
		char d[2048];
		asio::error_code error;
		auto&& tar = asio::ip::udp::endpoint(asio::ip::address::from_string("127.0.0.1"), 12345);
		auto now = std::chrono::system_clock::now();
		for (size_t i = 0; i < 100000; i++) {
			us.send_to(asio::buffer("asdf"), tar);
			us.receive_from(asio::buffer(d), rep, 0, error);
		}
		std::cout << std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now() - now).count() << std::endl;
		//ioc.run();
	}
	catch (std::exception& e) {
		std::cerr << e.what() << std::endl;
	}
	return 0;
}
//us.async_receive_from(asio::buffer(d), rep, [&](const asio::error_code& e, size_t recvLen) {
//	std::cout << "e = " << e << ", recvLen = " << recvLen << std::endl;
//});
