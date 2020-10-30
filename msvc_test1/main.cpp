#include "xx_ptr.h"
#include <iostream>
#include <vector>

struct Foo : xx::PtrBase {
    std::vector<xx::Weak<Foo>> parent;
    std::vector<xx::Shared<Foo>> childs;
    Foo() {
        std::cout << "Foo" << std::endl;
    }

    void Test() {
        std::cout << "Test" << std::endl;
    }

    ~Foo() override {
        std::cout << "~Foo" << std::endl;
    }
};

int main() {
    xx::Shared<Foo> sf;
    sf.Make();
    sf->childs.push_back(xx::MakeShared<Foo>());
    xx::Weak<Foo> wf(sf);
    xx::Weak<Foo> wf2;
    wf2 = sf;
    sf.Reset();
    auto&& sf2 = wf.Lock();
    std::cout << sf2.Empty() << std::endl;
    std::cout << wf.useCount() << std::endl;
    std::cout << wf2.refCount() << std::endl;
    wf.Reset();
    std::cout << wf.useCount() << std::endl;
    std::cout << wf2.refCount() << std::endl;
    wf2.Reset();
    std::cout << wf.useCount() << std::endl;
    std::cout << wf2.refCount() << std::endl;

    return 0;
}


//#include <iostream>
//#include <string>
//#include <memory>
//
//int main() {
//	auto len = 1024LL * 1024 * 1024 * 5;
//	auto a = new uint8_t[len];// { 0 };
//	size_t count = 0;
//	for (size_t i = 0; i < len; i += 4096) {
//		count += a[i];
//	}
//	std::cout << count << std::endl;
//	std::cin.get();
//	return 0;
//}


//#include <iostream>
//#include <string>
//#include <memory>
//#include <asio.hpp>
//
//int main() {
//	try {
//		char d[2048];
//		asio::io_context ioc(1);
//		asio::ip::udp::socket us(ioc, asio::ip::udp::endpoint(asio::ip::udp::v4(), 12345));
//		asio::ip::udp::endpoint ep;
//		std::function<void(const asio::error_code& e, size_t recvLen)> f;
//		f = [&](auto&& e, auto&& recvLen) {
//			if (!e) {
//				if (recvLen) {
//					us.send_to(asio::buffer(d, recvLen), ep);
//					//us.async_send_to(asio::buffer(d, recvLen), ep, [&](const asio::error_code& e, size_t sentLen) {
//					//	us.async_receive_from(asio::buffer(d), ep, f);
//					//});
//				}
//				//else
//				us.async_receive_from(asio::buffer(d), ep, f);
//			}
//		};
//		f({}, 0);
//		ioc.run();
//	}
//	catch (std::exception& e) {
//		std::cerr << e.what() << std::endl;
//	}
//	return 0;
//}



//// test client
//#include <iostream>
//#include <string>
//#include <memory>
//#include <chrono>
//#include <asio.hpp>
//
////socket.non_blocking(true);
////size_t len = 0;
////error = boost::asio::error::would_block;
////while (error == boost::asio::error::would_block)
//////do other things here like go and make coffee
////len = socket.receive_from(boost::asio::buffer(recv_buf), sender_endpoint, 0, error);
////std::cout.write(recv_buf.data(), len);
//
//int main() {
//    std::cout << "begin" << std::endl;
//    try {
//        asio::io_context ioc;
//        asio::ip::udp::socket us(ioc, asio::ip::udp::endpoint(asio::ip::udp::v4(), 0));
//        us.non_blocking(true);
//        std::cout << "asdf" << std::endl;
//        char d[2048];
//
//        auto &&tar = asio::ip::udp::endpoint(asio::ip::address::from_string("10.0.0.13"), 12333);
//        us.send_to(asio::buffer("asdf", 4), tar);
//        std::cout << "qwer" << std::endl;
//        size_t len = 0;
//        asio::error_code error;
//        asio::ip::udp::endpoint rep;
//        for (int i = 0; i < 10; ++i) {
//            len = us.receive_from(asio::buffer(d), rep, 0, error);
//            if (len) break;
//            //else if (error != asio::error::would_block)
//            std::this_thread::sleep_for(std::chrono::milliseconds(100));
//            std::cout << "i = " << i << std::endl;
//        }
//
//        std::cout << len << std::endl;
//        //ioc.run();
//    }
//    catch (std::exception &e) {
//        std::cerr << e.what() << std::endl;
//    }
//    std::cout << "end" << std::endl;
//    return 0;
//}
//////us.async_receive_from(asio::buffer(d), rep, [&](const asio::error_code& e, size_t recvLen) {
//////	std::cout << "e = " << e << ", recvLen = " << recvLen << std::endl;
//////});








//#include <cstdlib>
//#include <iostream>
//#include <functional>
//#include <asio.hpp>
//
//using asio::ip::udp;
//using namespace std::placeholders;
//
//struct server {
//	server(asio::io_service& io_service, short port)
//		: io_service_(io_service),
//		socket_(io_service, udp::endpoint(udp::v4(), port)) {
//		set_receive_handle();
//	}
//	void set_receive_handle() {
//		socket_.async_receive_from(
//			asio::buffer(data_, max_length), sender_endpoint_,
//			std::bind(&server::handle_receive_from, this, _1, _2));
//	}
//	void handle_receive_from(const asio::error_code& error, size_t bytes_recvd) {
//		if (!error && bytes_recvd > 0) {
//			socket_.async_send_to(
//				asio::buffer(data_, bytes_recvd), sender_endpoint_,
//				bind(&server::handle_send_to, this, _1, _2));
//		}
//		else {
//			set_receive_handle();
//		}
//	}
//	void handle_send_to(const asio::error_code& /*error*/, size_t /*bytes_sent*/) {
//		set_receive_handle();
//	}
//private:
//	asio::io_service& io_service_;
//	udp::socket socket_;
//	udp::endpoint sender_endpoint_;
//	enum { max_length = 1024 };
//	char data_[max_length];
//};
//
//int main(int argc, char* argv[]) {
//	try {
//		asio::io_service io_service;
//		server s(io_service, 12345);
//		io_service.run();
//	}
//	catch (std::exception& e) {
//		std::cerr << "Exception: " << e.what() << "\n";
//	}
//	return 0;
//}







//#include <array>
//#include <asio.hpp>
//#include <iostream>
//#include <thread>
//
//using asio::ip::udp;
//constexpr size_t buffer_size = 4096;
//
//template <typename Storage>
//class callback
//{
//public:
//
//	Storage buffer;
//	udp::endpoint sender_endpoint;
//
//	/* this function is the shim to access the async callback *with* data */
//	template <typename Callback>
//	auto operator()(const asio::error_code& ec, std::size_t bytes, Callback callback) const
//	{
//		auto buffer_begin = std::cbegin(buffer), buffer_end = std::cend(buffer);
//		return callback(sender_endpoint, ec, bytes, buffer_begin,
//			buffer_begin + std::min((std::size_t)std::max(0, (int)std::distance(buffer_begin, buffer_end)), bytes));
//	}
//};
//
//// just a demo callback with access to all the data. *This* is the target for all the surrounding effort!
//template <typename Endpoint, typename Iterator>
//void
//print_to_cout(const Endpoint& endpoint, const asio::error_code& ec, std::size_t bytes, Iterator begin, Iterator end)
//{
//	std::cout << "thread: " << std::this_thread::get_id() << ": Buffer: " << std::string(begin, end) << ", error code: " << ec << ", with " << bytes << " bytes, from endpoint " << endpoint
//		<< std::endl;
//}
//
//class server
//{
//public:
//
//	// the storage for each callback has 4k, we have 10 of them�� should be enough
//	using buffer_t = std::array<char, buffer_size>;
//	using callback_list_t = std::array<callback<buffer_t>, 3>;
//
//private:
//
//	callback_list_t m_callback_list;
//	udp::socket m_socket;
//
//public:
//
//	server(asio::io_service& io_service, short port)
//		: m_socket(io_service, udp::endpoint(udp::v4(), port))
//	{
//		do_receive(m_socket, m_callback_list.begin(), m_callback_list.begin(), m_callback_list.end());
//	}
//
//	/* this is an exercise to pass the data to async_receive_from *without* capturing "this". Since we might call this
//	   recursively, we don't want to rely on "this" pointer. */
//	static void do_receive(udp::socket& socket, callback_list_t::iterator current_callback, callback_list_t::iterator callback_begin,
//		callback_list_t::const_iterator callback_end)
//	{
//		// via the current_callback mechanism, we are able to compartmentalize the storage for each callback
//		socket.async_receive_from(
//			asio::buffer(current_callback->buffer, buffer_size - 1), current_callback->sender_endpoint,
//			[&socket, current_callback, callback_begin, callback_end](const asio::error_code& ec, std::size_t bytes_recvd) {
//				// now, move on to the next storage-and-callback - with wraparound!
//				auto next_callback = current_callback + 1;
//				do_receive(socket, (next_callback != callback_end) ? next_callback : callback_begin, callback_begin, callback_end);
//
//				// call our handler. we need to be sure to have access to the data - but we want to call do_receive as soon as possible!
//				// if this takes too long, you might want to ship this off to a separate thead. Subsequent calls to do_receive are
//				// *not* depending on data in current_callback!
//				(*current_callback)(ec, bytes_recvd, print_to_cout<decltype(current_callback->sender_endpoint), buffer_t::const_iterator>);
//				// since this is an echo service, call do_send() with the same data
//				do_send(socket, current_callback, bytes_recvd);
//			});
//	}
//
//	// do_send is much easier than do_receive - we *have* all the data, we don't need to advance the callback��
//	static void do_send(udp::socket& socket, callback_list_t::iterator current_callback, std::size_t length)
//	{
//		socket.async_send_to(asio::buffer(current_callback->buffer, std::min(length, buffer_size - 1)), current_callback->sender_endpoint,
//			[current_callback](const asio::error_code& ec, std::size_t bytes_sent) {
//				(*current_callback)(ec, bytes_sent,
//					print_to_cout<decltype(current_callback->sender_endpoint), buffer_t::const_iterator>);
//			});
//	}
//};
//
//int
//main(int argc, char* argv[])
//{
//	//if (argc != 2) {
//	//    std::cerr << "Usage: async_udp_echo_server <port>" << std::endl;
//	//    return 1;
//	//}
//
//	asio::io_service io_service;
//
//	server s(io_service, 12345);//std::atoi(argv[1]));
//
//	// now to the fun part - let's call the io_service several times
//	std::thread thread1([&io_service]() { io_service.run(); });
//	std::thread thread2([&io_service]() { io_service.run(); });
//	std::thread thread3([&io_service]() { io_service.run(); });
//	std::thread thread4([&io_service]() { io_service.run(); });
//	thread1.join();
//	thread2.join();
//	thread3.join();
//	thread4.join();
//	return 0;
//}





//#include <iostream>
//#include <string>
//#include <memory>
//#include <asio.hpp>
//using namespace std::placeholders;
//
//struct tcp_connection : public std::enable_shared_from_this<tcp_connection> {
//	typedef std::shared_ptr<tcp_connection> pointer;
//
//	static pointer create(asio::io_context& io_context) {
//		return pointer(new tcp_connection(io_context));
//	}
//	asio::ip::tcp::socket& socket() {
//		return socket_;
//	}
//	void start() {
//		asio::async_write(socket_, asio::buffer("abcdefg"),
//			bind(&tcp_connection::handle_write, shared_from_this(), _1, _2));
//	}
//private:
//	tcp_connection(asio::io_context& io_context) : socket_(io_context) {}
//	void handle_write(const asio::error_code& /*error*/, size_t /*bytes_transferred*/) {}
//	asio::ip::tcp::socket socket_;
//};
//
//struct tcp_server {
//	tcp_server(asio::io_context& io_context) : io_context_(io_context),
//		acceptor_(io_context, asio::ip::tcp::endpoint(asio::ip::tcp::v4(), 12345)) {
//		start_accept();
//	}
//private:
//	void start_accept() {
//		auto new_connection = tcp_connection::create(io_context_);
//		acceptor_.async_accept(new_connection->socket(),
//			bind(&tcp_server::handle_accept, this, new_connection, _1));
//	}
//	void handle_accept(tcp_connection::pointer new_connection, const asio::error_code& error) {
//		if (!error) {
//			new_connection->start();
//		}
//		start_accept();
//	}
//	asio::io_context& io_context_;
//	asio::ip::tcp::acceptor acceptor_;
//};
//
//int main() {
//	try {
//		asio::io_context io_context;
//		tcp_server server(io_context);
//		io_context.run();
//	}
//	catch (std::exception& e) {
//		std::cerr << e.what() << std::endl;
//	}
//	return 0;
//}
