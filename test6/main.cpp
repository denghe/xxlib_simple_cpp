#include "xx_epoll_kcp.h"
#include <thread>
#include <atomic>

namespace EP = xx::Epoll;

sockaddr_in6 addr{};
int numThreads = 0;
int numUdps = 0;
std::atomic<uint64_t> counter = 0;
char const* dataForSend = nullptr;
size_t dataLenForSend = 0;

struct Peer : EP::UdpPeer {
    explicit Peer(std::shared_ptr<EP::Context> const &ec) : EP::UdpPeer(ec, -1) {
        if (int r = MakeFD()) {
            throw std::runtime_error("listen failed.");
        }
        SendTo(::addr, dataForSend, dataLenForSend);
    }

    void Receive(char const *const &buf, size_t const &len) override;
};

struct Client : EP::Context {
    std::vector<std::shared_ptr<Peer>> peers;

    int Run() override {
        xx::ScopeGuard sg1([&] {
            peers.clear();
            holdItems.clear();
            assert(shared_from_this().use_count() == 2);
        });
        for (int i = 0; i < numUdps; ++i) {
            peers.emplace_back(xx::Make<Peer>(shared_from_this()));
        }
        return this->EP::Context::Run();
    }
};

inline void Peer::Receive(char const *const &buf, size_t const &len) {
    ++counter;
    Send(buf, len);
}

int main(int argc, char const *argv[]) {
    if (argc < 6) {
        throw std::logic_error("need 5 args: numThreads  numUdps  ip  port  content");
    }
    xx::Convert(argv[1], numThreads);
    if (numThreads < 1 || numThreads > 100) {
        throw std::logic_error("invalid numThreads?");
    }

    xx::Convert(argv[2], numUdps);
    if (numUdps < 1 || numUdps > 100) {
        throw std::logic_error("invalid numUdps?");
    }

    int port = 0;
    xx::Convert(argv[4], port);
    if (int r = EP::FillAddress(argv[3], port, addr)) {
        throw std::logic_error("invalid ip?");
    }

    dataForSend = argv[5];
    dataLenForSend = strlen(dataForSend);

    std::vector<std::thread> ts;
    ts.reserve(numThreads);
    for (int i = 0; i < numThreads; ++i) {
        ts.emplace_back([] { xx::Make<Client>()->Run(); });
    }
    xx::CoutN("running... num threads = ", numThreads);

    std::thread tc([&] {
        while (true) {
            std::cout << counter << std::endl;
            counter = 0;
            std::this_thread::sleep_for(std::chrono::seconds(1));
        }
    });

    for (auto &&t : ts) {
        t.join();
    }
    return 0;
}
