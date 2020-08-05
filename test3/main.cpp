#include "xx_epoll_kcp.h"
#include <thread>
namespace EP = xx::Epoll;

struct Peer : EP::UdpPeer {
    Peer(std::shared_ptr<EP::Context> const &ec, int const& port) : EP::UdpPeer(ec, -1) {
        if (int r = Listen(port, "0.0.0.0")) {
            throw std::runtime_error("listen failed.");
        }
    }
    void Receive(char const *const &buf, size_t const &len) override {
        ++counter;
        Send(buf, len);
    }
    uint64_t counter = 0;
};

struct Server : EP::Context {
    std::vector<std::shared_ptr<Peer>> peers;

    int Run() override {
        xx::ScopeGuard sg1([&]{
            peers.clear();
            holdItems.clear();
            assert(shared_from_this().use_count() == 2);
        });
        SetFrameRate(1);
        for (int i = 0; i < 1; ++i) {
            peers.emplace_back(xx::Make<Peer>(shared_from_this(), 5555/* + i*/));
        }
        return this->EP::Context::Run();
    }

    int FrameUpdate() override {
        uint64_t counter = 0;
        for(auto&& p : peers) {
            counter += p->counter;
            p->counter = 0;
        }
        xx::CoutN(counter);
        return 0;
    }
};

int main() {
    int n = 1;
    std::vector<std::thread> ts;
    for (int i = 0; i < n; ++i) {
        ts.emplace_back([] { xx::Make<Server>()->Run(); });
    }
    xx::CoutN("running... num threads = ", n);
    for (auto &&t : ts) {
        t.join();
    }
    return 0;
}