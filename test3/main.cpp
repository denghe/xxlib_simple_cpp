#include "xx_epoll_kcp.h"
#include <thread>

namespace EP = xx::Epoll;

struct Peer : EP::KcpPeer {
    using EP::KcpPeer::KcpPeer;

    void Receive() override {
        Send(recv.buf, recv.len);       // echo back
        recv.Clear();
        SetTimeoutSeconds(10);
        ++counter;
    }
    uint64_t counter = 0;

    bool Close(int const &reason, char const *const &desc) override {
        if (this->EP::KcpPeer::Close(reason, desc)) {
            DelayUnhold();
            return true;
        }
        return false;
    }
};

struct Listener : EP::KcpListener<Peer> {
    using EP::KcpListener<Peer>::KcpListener;

    void Accept(std::shared_ptr<Peer> const &peer) override {
        peer->Hold();
        peer->SetTimeoutSeconds(15);
    }
};

struct Server : EP::Context {
    using EP::Context::Context;
    std::shared_ptr<Listener> listener;
    std::shared_ptr<EP::GenericTimer> timer;

    int Run() override {
        xx::ScopeGuard sg1([&] {
            timer.reset();
            listener->Close(__LINE__, "exit");
            holdItems.clear();
            assert(shared_from_this().use_count() == 2);
        });

        xx::MakeTo(listener, shared_from_this());
        listener->readCountAtOnce = 500;
        listener->Listen(5555, nullptr, false, 1784 * 10000, 1784 * 10000);

        xx::MakeTo(timer, shared_from_this());
        timer->SetTimeoutSeconds(1);
        timer->onTimeout = [this] {
            uint64_t counter = 0;
            for (auto &&p :  listener->cps) {
                counter += ((Peer *) p.second)->counter;
                ((Peer *) p.second)->counter = 0;
            }
            xx::CoutN("listener->cps.size() = ", listener->cps.size(), " counter = ", counter);
            timer->SetTimeoutSeconds(1);
        };

        SetFrameRate(100);

        return this->EP::Context::Run();
    }
};

int main() {
    int n = 1;
    std::vector<std::thread> ts;
    for (int i = 0; i < n; ++i) {
        ts.emplace_back([] { xx::Make<Server>((1u << 16u))->Run(); });
    }
    xx::CoutN("running... num threads = ", n);
    for (auto &&t : ts) {
        t.join();
    }
    return 0;
}



//#include "xx_epoll_kcp.h"
//#include <thread>
//namespace EP = xx::Epoll;
//
//struct Peer : EP::UdpPeer {
//    Peer(std::shared_ptr<EP::Context> const &ec, int const& port) : EP::UdpPeer(ec, -1) {
//        if (int r = Listen(port, "0.0.0.0")) {
//            throw std::runtime_error("listen failed.");
//        }
//    }
//    void Receive(char const *const &buf, size_t const &len) override {
//        ++counter;
//        Send(buf, len);
//    }
//    uint64_t counter = 0;
//};
//
//struct Server : EP::Context {
//    std::vector<std::shared_ptr<Peer>> peers;
//
//    int Run() override {
//        xx::ScopeGuard sg1([&]{
//            peers.clear();
//            holdItems.clear();
//            assert(shared_from_this().use_count() == 2);
//        });
//        SetFrameRate(1);
//        for (int i = 0; i < 1; ++i) {
//            peers.emplace_back(xx::Make<Peer>(shared_from_this(), 5555/* + i*/));
//        }
//        return this->EP::Context::Run();
//    }
//
//    int FrameUpdate() override {
//        uint64_t counter = 0;
//        for(auto&& p : peers) {
//            counter += p->counter;
//            p->counter = 0;
//        }
//        xx::CoutN(counter);
//        return 0;
//    }
//};
//
//int main() {
//    int n = 1;
//    std::vector<std::thread> ts;
//    for (int i = 0; i < n; ++i) {
//        ts.emplace_back([] { xx::Make<Server>()->Run(); });
//    }
//    xx::CoutN("running... num threads = ", n);
//    for (auto &&t : ts) {
//        t.join();
//    }
//    return 0;
//}