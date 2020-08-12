#include "xx_epoll_kcp.h"
#include "xx_data_rw.h"
#include <thread>
#include <atomic>

namespace EP = xx::Epoll;

std::atomic<uint64_t> g_counter;
std::atomic<uint64_t> g_counter2;
int n = 1;
int nc = 200;
int port = 0;
sockaddr_in6 addr{};



struct Peer : EP::KcpPeer {
    using EP::KcpPeer::KcpPeer;
    xx::Data d;

    inline void Receive() override {
        // 取出指针备用
        auto buf = recv.buf;
        auto end = recv.buf + recv.len;
        uint32_t dataLen = 0;

        // 确保包头长度充足
        while (buf + sizeof(dataLen) <= end) {
            // 取长度
            dataLen = *(uint32_t *) buf;

            // 长度异常则断线退出( 不含地址? 超长? 256k 不够可以改长 )
            if (dataLen > 1024 * 256) {
                Close(-21, __LINESTR__" Peer Receive if (dataLen < sizeof(addr) || dataLen > 1024 * 256)");
                return;
            }

            // 数据未接收完 就 跳出
            if (buf + sizeof(dataLen) + dataLen > end) break;

            // 如果回包内容正确, 就 计数 发新包 续命
            if ( sizeof(dataLen) + dataLen == d.len && memcmp(buf, d.buf, d.len) == 0) {
                ++g_counter;
                Send();
                SetTimeoutSeconds(10);
            }

            // 跳到下一个包的开头
            buf += sizeof(dataLen) + dataLen;
        }

        // 移除掉已处理的数据( 将后面剩下的数据移动到头部 )
        recv.RemoveFront(buf - recv.buf);
    }

    inline bool Close(int const &reason, char const *const &desc) override {
        --g_counter2;
        if (this->EP::KcpPeer::Close(reason, desc)) {
            DelayUnhold();
            return true;
        }
        return false;
    }

    // 用来发送的内容. 收到回包时对比一下
    uint64_t msg = 0;
    inline void Send() {
        ++msg;
        xx::DataWriter dw(d);
        d.len = 0;
        dw.WriteFixed((uint32_t) 0);
        dw.WriteFixed((uint32_t) 0xFFFFFFFF);
        dw.WriteFixed(msg);
        *(uint32_t *) d.buf = d.len - sizeof(uint32_t);
        this->EP::KcpPeer::Send(d.buf, d.len);
        Flush();
    }
};

struct Dialer : EP::KcpDialer<Peer> {
    using EP::KcpDialer<Peer>::KcpDialer;

    inline void Connect(std::shared_ptr<Peer> const &peer) override {
        if (!peer) return; // 没连上
        ++g_counter2;
        peer->Hold();
        peer->SetTimeoutSeconds(10);
        peer->Send();
    }
};

struct Client : EP::Context {
    using EP::Context::Context;
    std::shared_ptr<Dialer> dialer;
    std::shared_ptr<EP::GenericTimer> timer;
    bool needSendFirstPackage = true;

    int Run() override {
        xx::ScopeGuard sg1([&] {
            timer.reset();
            dialer->Stop();
            dialer.reset();
            holdItems.clear();
            assert(shared_from_this().use_count() == 2);
        });

        xx::MakeTo(timer, shared_from_this());
        timer->onTimeout = [this] {
            if (!dialer) {
                xx::MakeTo(dialer, shared_from_this());
                if (int r = dialer->MakeFD(0, nullptr, false, 1784 * nc, 1784 * nc)) {
                    dialer.reset();
                    xx::CoutN("MakeFD error. r = ", r);
                } else {
                    dialer->needSendFirstPackage = needSendFirstPackage;
                    dialer->readCountAtOnce = nc;
                    dialer->AddAddress(addr);
                }
            }
            if (dialer && !dialer->Busy() && dialer->cps.size() < nc) {
                dialer->DialSeconds(2);
            }
            timer->SetTimeout(1);
        };
        timer->SetTimeout(1);

        SetFrameRate(100);
        return this->EP::Context::Run();
    }
};

int main(int argc, char const *argv[]) {
    if (argc < 6) {
        throw std::logic_error("need 5 args: numThreads  numSockets   ip  port   needSendFirstPackage(0/1)");
    }
    xx::Convert(argv[1], n);
    if (n < 1 || n > 100) {
        throw std::logic_error("invalid numThreads?");
    }
    xx::Convert(argv[2], nc);
    if (nc < 1 || nc > 200) {
        throw std::logic_error("invalid numSockets?");
    }
    xx::Convert(argv[4], port);
    if (int r = EP::FillAddress(argv[3], port, addr)) {
        throw std::logic_error("invalid ip?");
    }

    bool needSendFirstPackage = (argv[5][0] == '1');

    std::vector<std::thread> ts;
    for (int i = 0; i < n; ++i) {
        ts.emplace_back([&, i = i] {
            auto &&c = xx::Make<Client>();
            c->needSendFirstPackage = needSendFirstPackage;
            c->Run();
        });
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
        xx::Cout(".");
        xx::CoutFlush();
    }

    std::thread timerThread([&] {
        while (true) {
            std::cout << g_counter2 << ", " << g_counter << std::endl;
            g_counter = 0;
            std::this_thread::sleep_for(std::chrono::seconds(1));
        }
    });

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
