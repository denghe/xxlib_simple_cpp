#include "xx_epoll_kcp.h"
#include <thread>
namespace EP = xx::Epoll;

struct Peer : EP::KcpPeer {
    using BaseType = EP::KcpPeer;
    using BaseType::BaseType;

    // 继续将数据发出
    void Receive() override {
        Send(recv.buf, recv.len);
        recv.Clear();
        SetTimeoutSeconds(10);
    }

    // 从 Client.peer 移除并 DelayUnhold
    bool Close(int const &reason, char const *const &desc) override;
};

struct Dialer : EP::KcpDialer<Peer> {
    using EP::KcpDialer<Peer>::KcpDialer;

    // 如果 peer != nullptr 则放入 Client.peer 并 Hold
    void Connect(std::shared_ptr<Peer> const &peer) override;
};

struct Client : EP::Context {
    using EP::Context::Context;
    std::shared_ptr<Dialer> dialer;
    std::shared_ptr<EP::GenericTimer> timer;
    int port = 0;

    int Run() override {
        xx::ScopeGuard sg1([&] {
            timer.reset();
            dialer->Stop();
            dialer.reset();
            holdItems.clear();
            assert(shared_from_this().use_count() == 2);
        });

        xx::MakeTo(timer, shared_from_this());
        timer->onTimeout = [this]{
            if (!dialer) {
                xx::MakeTo(dialer, shared_from_this(), port);
                dialer->AddAddress("192.168.1.235", 5555);
            }
            if (!dialer->Busy() && dialer->cps.size() < 10000) {
                dialer->DialSeconds(2);
                //xx::CoutN("dial....");
            }
            timer->SetTimeout(1);
        };
        timer->SetTimeout(1);

        SetFrameRate(200);

        return this->EP::Context::Run();
    }
};

void Dialer::Connect(std::shared_ptr<Peer> const &peer) {
    if (!peer) return; // 没连上
    assert(!((Client*)&*ec)->peer);
    peer->Hold();
    peer->SetTimeoutSeconds(10);
    peer->Send("asdf", 4);
    //xx::CoutN("connected to ", addr);
}

bool Peer::Close(int const &reason, char const *const &desc) {
    if (this->BaseType::Close(reason, desc)) {
        DelayUnhold();
        return true;
    }
    return false;
}

int main() {
    int n = 5;
    std::vector<std::thread> ts;
    for (int i = 0; i < n; ++i) {
        ts.emplace_back([i=i] {
            auto&& c = xx::Make<Client>();
            //c->port = 10000 + i;
            c->Run();
        });
        std::this_thread::sleep_for(std::chrono::milliseconds (100));
        xx::Cout(".");
        xx::CoutFlush();
    }
    xx::CoutN("running... num threads = ", n);
    for (auto &&t : ts) {
        t.join();
    }
    return 0;
}


//#include "xx_epoll_kcp.h"
//#include <thread>
//
//namespace EP = xx::Epoll;
//
//struct Peer : EP::UdpPeer {
//    Peer(std::shared_ptr<EP::Context> const &ec, int const &port) : EP::UdpPeer(ec, -1) {
//        if (int r = Listen(0, "0.0.0.0")) {
//            throw std::runtime_error("listen failed.");
//        }
//        EP::FillAddress("192.168.1.235", port, addr);
//        Send("asdf", 5);
//    }
//
//    void Receive(char const *const &buf, size_t const &len) override;
//};
//
//struct Client : EP::Context {
//    std::vector<std::shared_ptr<Peer>> peers;
//    uint64_t counter = 0;
//
//    int Run() override {
//        xx::ScopeGuard sg1([&] {
//            peers.clear();
//            holdItems.clear();
//            assert(shared_from_this().use_count() == 2);
//        });
//        SetFrameRate(1);
//        for (int i = 0; i < 1; ++i) {
//            peers.emplace_back(xx::Make<Peer>(shared_from_this(), 5555));
//        }
//        return this->EP::Context::Run();
//    }
//
//    int FrameUpdate() override {
//        xx::CoutN(counter);
//        counter = 0;
//        return 0;
//    }
//};
//
//inline void Peer::Receive(char const *const &buf, size_t const &len) {
//    ++((Client *) &*ec)->counter;
//    Send(buf, len);
//}
//
//int main() {
//    int n = 1;
//    std::vector<std::thread> ts;
//    for (int i = 0; i < n; ++i) {
//        ts.emplace_back([] { xx::Make<Client>()->Run(); });
//    }
//    xx::CoutN("running... num threads = ", n);
//    for (auto &&t : ts) {
//        t.join();
//    }
//    return 0;
//}
