#include <iostream>
#include <string>
#include <memory>
#include <asio.hpp>

int main() {
	try {
		char d[1024];
		asio::io_context ioc;
		asio::ip::udp::resolver resolver(ioc);
		asio::ip::udp::socket us(ioc, asio::ip::udp::endpoint(asio::ip::udp::v4(), 0));
		asio::ip::udp::endpoint rep;
		us.async_receive_from(asio::buffer(d), rep, [&](const asio::error_code& e, size_t recvLen) {
			std::cout << "e = " << e << ", recvLen = " << recvLen << std::endl;
		});
		auto&& ep = resolver.resolve("127.0.0.1", "12345")->endpoint();
		std::cout << "ep = " << ep << std::endl;
		us.send_to(asio::buffer("asdf"), ep);
		// todo: Í¬²½ÊÕ for performance test
		ioc.run();
	}
	catch (std::exception& e) {
		std::cerr << e.what() << std::endl;
	}
	return 0;
}
