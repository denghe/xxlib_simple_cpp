#include "xx_epoll_kcp.h"
#include <thread>

namespace xx::Epoll {

    // 这东西用起来要非常小心：闭包上下文可能加持指针，可能引用到野指针，可能析构对象导致 this 失效
    struct GenericTimer : Timer {
        using Timer::Timer;
        std::function<void()> onTimeout;

        inline void Timeout() override {
            onTimeout();
        }
    };

    template<typename PeerType, class ENABLED = std::is_base_of<KcpPeer, PeerType>>
    struct KcpDialer : KcpBase {
        // 自己本身的 timer 已在基类用于每帧更新 cps
        GenericTimer timerForShake;
        GenericTimer timerForTimeout;

        // 初始化 timer
        explicit KcpDialer(std::shared_ptr<Context> const &ec);

        // 连接成功或失败(peer==nullptr)都会触发. 失败通常属于超时
        virtual void Connect(std::shared_ptr<PeerType> const &peer) = 0;

        // 向 addrs 追加地址. 如果地址转换错误将返回非 0
        int AddAddress(std::string const &ip, int const &port);

        // 开始拨号( 超时单位：帧 )。会遍历 addrs 为每个地址创建一个 TcpConn 连接
        // 保留先连接上的 socket fd, 创建 Peer 并触发 Connect 事件.
        // 如果超时，也触发 Connect，参数为 nullptr
        int Dial(int const &timeoutFrames);

        // 开始拨号( 超时单位：秒 )
        int DialSeconds(double const &timeoutSeconds);

        // 返回是否正在拨号
        bool Busy();

        // 停止拨号. 保留 addrs.
        void Stop();

    protected:
        // 存个空值备用 以方便返回引用
        std::shared_ptr<PeerType> emptyPeer;

        // 1. 判断收到的数据内容, 模拟握手，最后产生 KcpPeer
        // 2. 定位到 KcpPeer, Input 数据( 已连接状态 )
        void Receive(char const *const &buf, size_t const &len) override;

        // 基于每个目标 addr 生成的 连接上下文对象，里面有 serial, conv 啥的. 拨号完毕后会被清空
        uint32_t serialAutoInc = 0;
        std::vector<uint32_t> serials;

        // 要连的地址数组
        std::vector<sockaddr_in6> addrs;

        // 关闭 fd, 关闭所有子
        bool Close(int const &reason, char const *const &desc) override;
    };

    template<typename PeerType, class ENABLED>
    void KcpDialer<PeerType, ENABLED>::Receive(char const *const &buf, size_t const &len) {
        // 如果收到任意返回的 8 字节( serial + convId ), 如果地址对的上，就停止拨号握手相关 timer，创建 kcp 上下文并发送 1 0 0 0 0, 触发 Connect
        if (len == 8) {
            auto serial = *(uint32_t *) buf;
            auto convId = *(uint32_t *) (buf + 4);
            // 在拨号时产生的序号中查找. 找不到就忽略这个数据
            size_t i = 0;
            for (; i < serials.size(); ++i) {
                if (serials[i] == serial) break;
            }
            if (i == serials.size()) return;

            // 创建 peer
            auto &&peer = xx::Make<PeerType>(xx::As<KcpBase>(shared_from_this()), convId);
            // 放入容器( 这个容器不会加持 )
            cps[convId] = &*peer;
            // 填充地址( 就填充拨号用的，不必理会收到的 )
            memcpy(&peer->addr, &addrs[i], sizeof(addr));
            // 发 kcp 版握手包
            peer->Send("\1\0\0\0\0", 5);
            // 触发事件回调
            Connect(peer);
            // 如果 已Close 或 未持有 就短路出去
            if (!peer->Alive() || peer.use_count() == 1) return;
        }

        // 忽略长度小于 kcp 头的数据包 ( IKCP_OVERHEAD at ikcp.c )
        if (len < 24) return;
        // kcp 头 以 convId 打头
        auto convId = *(uint32_t *) buf;
        // 根据 conv 试定位到 peer. 找不到就忽略掉( 可能是上个连接的残留数据过来 )
        auto &&peerIter = cps.find(convId);
        // 找到就灌入数据
        if (peerIter != cps.end()) {
            // 将数据灌入 kcp. 进而可能触发 peer->Receive 进而可能 Close
            peerIter->second->Input(buf, len);
        }
    }

    template<typename PeerType, class ENABLED>
    KcpDialer<PeerType, ENABLED>::KcpDialer(std::shared_ptr<Context> const &ec) : KcpBase(ec) {
        // 初始化 fd
        if (Listen(0))
            throw std::runtime_error(__LINESTR__" KcpDialer KcpDialer can't create kcp dialer. Listen(0) failed.");

        // 初始化握手 timer
        timerForShake.onTimeout = [this] {
            // 如果 serials 不空（ 意味着正在拨号 ), 就每秒数次无脑向目标地址发送 serial
            for (size_t i = 0; i < serials.size(); ++i) {
                SendTo(addrs[i], &serials[i], 4);
            }
            timerForShake.SetTimeoutSeconds(0.2);
        };

        // 初始化拨号超时 timer
        timerForTimeout.onTimeout = [this] {
            Stop();
            Connect(emptyPeer);
        };
    }

    template<typename PeerType, class ENABLED>
    void KcpDialer<PeerType, ENABLED>::Stop() {
        serials.clear();
        timerForShake.SetTimeout(0);
        timerForTimeout.SetTimeout(0);
    }

    template<typename PeerType, class ENABLED>
    bool KcpDialer<PeerType, ENABLED>::Busy() {
        return !serials.empty();
    }

    template<typename PeerType, class ENABLED>
    int KcpDialer<PeerType, ENABLED>::Dial(int const &timeoutFrames) {
        Stop();
        for (auto &&a : addrs) {
            serials.emplace_back(++serialAutoInc);
        }
        timerForShake.SetTimeoutSeconds(0.2);
        timerForTimeout.SetTimeout(timeoutFrames);
    }

    template<typename PeerType, class ENABLED>
    int KcpDialer<PeerType, ENABLED>::DialSeconds(double const &timeoutSeconds) {
        return Dial(ec->SecondsToFrames(timeoutSeconds));
    }

    template<typename PeerType, class ENABLED>
    int KcpDialer<PeerType, ENABLED>::AddAddress(std::string const &ip, int const &port) {
        auto &&a = addrs.emplace_back();
        if (int r = FillAddress(ip, port, a)) {
            addrs.pop_back();
            return r;
        }
        return 0;
    }

    template<typename PeerType, class ENABLED>
    bool KcpDialer<PeerType, ENABLED>::Close(int const &reason, char const *const &desc) {
        // 防重入 顺便关 fd
        if (!this->KcpBase::Close(reason, desc)) return false;
        // 关闭所有虚拟 peer
        CloseChilds(reason, desc);
        // 从容器变量移除
        DelayUnhold();
        return true;
    }
}


namespace EP = xx::Epoll;

struct Peer : EP::KcpPeer {
    using EP::KcpPeer::KcpPeer;

    void Receive() override {
        Send(recv.buf, recv.len);
        recv.Clear();
    }
};

struct Dialer : EP::KcpDialer<Peer> {
    using EP::KcpDialer<Peer>::KcpDialer;

    // 如果 peer != nullptr 则连接已建立, 搞事
    void Connect(std::shared_ptr<Peer> const &peer) override;
};

struct Client : EP::Context {
    using EP::Context::Context;
    std::shared_ptr<Dialer> dialer;
    // todo: timer for auto dial

    int Run() override {
        xx::ScopeGuard sg1([&] {
            dialer->Stop();
            dialer.reset();
            holdItems.clear();
            assert(shared_from_this().use_count() == 2);
        });
        SetFrameRate(100);
        return this->EP::Context::Run();
    }
};

void Dialer::Connect(std::shared_ptr<Peer> const &peer) {
    if (!peer) return; // 没连上
    peer->Hold();
    peer->SetTimeoutSeconds(30);
    peer->Send("asdf", 4);
    xx::CoutN("connected to ", addr);
}

int main() {
    int n = 1;
    std::vector<std::thread> ts;
    for (int i = 0; i < n; ++i) {
        ts.emplace_back([] { xx::Make<Client>()->Run(); });
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
