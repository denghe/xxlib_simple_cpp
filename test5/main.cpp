#include <memory>
#include <thread>
#include <vector>
#include <mutex>
#include <unordered_map>
#include <tuple>
#include <atomic>

#include "xx_net.h"
#include "xx_looper.h"
#include "xx_data.h"
#include "xx_chrono.h"
#include "ikcp.h"

/*

多线程同时监听同一个 udp port, 阻塞收取
如果是 握手, 投递到一个独立线程处理. 使用来源 fd & addr 发回携带 conv 的握手回包
如果是 kcp 包, 根据 conv 分组投递到相应处理线程, 使用来源 fd & addr 通信

clients -----> [udp reader]\
clients -----> [udp reader]-+[shake handler]
clients -----> [udp reader]/

                           /[kcp handler][ peer ... ]
clients -----> [udp reader]-[kcp handler][ peer ... ]
                           \[kcp handler][ peer ... ]

shake handler, kcp handler 都有自己的内循环
shake handler 利用循环处理超时对象，频率较低，一秒一次
kcp handler 利用循环调用 kcp update, 以迎合 kcp 算法需求，频率较高，一秒 100 次
udp reader 与 kcp handler 通过带锁队列传递数据包

*/

/**********************************************************************************************************************/
// FdAddrData
/**********************************************************************************************************************/

// 投递到 Kcp线程池的数据
struct FdAddrData {
    // 来源 fd
    int fd = 0;
    // 来源地址
    sockaddr_in6 addr{};
    // convId
    uint32_t conv = 0;
    // 数据
    xx::Data data;

    FdAddrData() = default;

    FdAddrData(FdAddrData const &) = delete;

    FdAddrData &operator=(FdAddrData const &) = delete;

    FdAddrData(FdAddrData &&o) noexcept: data(std::move(o.data)) {
        std::swap(fd, o.fd);
        std::swap(conv, o.conv);
        std::swap(addr, o.addr);
    }

    FdAddrData &operator=(FdAddrData &&o) noexcept {
        std::swap(fd, o.fd);
        std::swap(addr, o.addr);
        std::swap(conv, o.conv);
        std::swap(data, o.data);
        return *this;
    }
};


// 跨线程大环境
struct Env;

/**********************************************************************************************************************/
// UdpReader
/**********************************************************************************************************************/

// UDP 死循环无脑阻塞读取线程上下文
struct UdpReader {
    UdpReader() = default;

    UdpReader(UdpReader const &) = delete;

    UdpReader &operator=(UdpReader const &) = delete;

    UdpReader(UdpReader &&o) noexcept;

    UdpReader &operator=(UdpReader &&o) noexcept;

    Env *env = nullptr;

    // 初始化 udp fd
    explicit UdpReader(Env *const &env);

    ~UdpReader();

protected:
    friend Env;

    // udp fd
    int fd = 0;

    // 执行入口
    void Run();

    // 关闭 fd 以令 Run 退出
    void Close();
};


/**********************************************************************************************************************/
// ShakeHandler
/**********************************************************************************************************************/

// 握手线程上下文( 执行入口 Run 在基类 )
struct ShakeHandler : xx::Looper::Context {
    using xx::Looper::Context::Context;
    Env *env = nullptr;

protected:
    // 外部函数 lock 用
    std::mutex mtx;

    // 带超时的握手信息字典 key: ip:port   value: conv, nowMS
    std::unordered_map<std::string, std::pair<uint32_t, int64_t>> shakes;

    // 自增生成 conv 用
    uint32_t convId = 0;

    // 扫描超时的逻辑
    int FrameUpdate() override;

    // 根据 ip_port 返回一个 convId. 如果已存在就返回旧的。否则就生成
    uint32_t TryAdd(sockaddr_in6 const &addr, int const &timeoutMS = 3000);

public:
    // 4 字节握手包处理函数( 会使用来源 fd 发回 8 字节附带 convId )
    void HandleShake(int const &fd, sockaddr_in6 const &addr, uint32_t const &serial);

    // 判断 ip_port 是否在握手列表中. 如果在 且 conv 对的上, 就移除并返回 true
    bool FindAndRemove(sockaddr_in6 const &addr, uint32_t const &conv);
};


struct KcpPeer;

/**********************************************************************************************************************/
// KcpHandler
/**********************************************************************************************************************/

struct KcpHandler : xx::Looper::Context {
    using xx::Looper::Context::Context;
    Env *env = nullptr;
    int id = 0;

protected:
    friend struct KcpPeer;

    // for 前后台队列交换 lock
    std::mutex mtx;

    // 前后台数据接收队列
    std::vector<FdAddrData> fads1, fads2;

    // key: conv。KcpPeer Close 时从该字典移除 key
    std::unordered_map<uint32_t, KcpPeer *> cps;

    // kcp update
    int FrameUpdate() override;

    // kcp input from fads2
    int Wait(int const &ms) override;

    // create KcpPeer
    void Accept(std::shared_ptr<KcpPeer> const &peer);

public:
    // 往 fads1 锁定插入数据
    void Push(FdAddrData &&fad);
};


/**********************************************************************************************************************/
// KcpPeer
/**********************************************************************************************************************/

struct KcpPeer : xx::Looper::Item {
    // 用于收发数据的物理 udp fd
    int fd = 0;
    // 对方的 addr( owner 收到数据时会填充. SendTo 时用到 )
    sockaddr_in6 addr{};
    // 收数据用堆积容器( Receive 里访问它来处理收到的数据 )
    xx::Data recv;

    // kcp 相关上下文
    ikcpcb *kcp = nullptr;
    uint32_t conv = 0;
    int64_t createMS = 0;
    uint32_t nextUpdateMS = 0;

    // for easy use
    inline KcpHandler *GetKcpHandler() { return (KcpHandler *) ctx; }

    // 初始化 kcp 相关上下文
    KcpPeer(KcpHandler *const &looper, int const &fd, uint32_t const &conv);

    // 回收 kcp 并延迟自杀( 非析构 )
    bool Close(int const &reason, char const *const &desc) override;

    // Close(0)
    ~KcpPeer() override;

    // 被 ep 调用. 受帧循环驱动. 帧率越高, kcp 工作效果越好. 典型的频率为 100 fps
    void UpdateKcpLogic();

    // 被 looper 调用. 塞数据到 kcp
    void Input(char const *const &buf, size_t const &len_, bool isFirst = false);

    // 传进发送队列( 如果 !kcp 则忽略 )
    int Send(char const *const &buf, size_t const &len);

    // 立刻开始发送
    int Flush();

    // kcp != null
    bool Alive() const;

    // 接收数据处理
    void Receive();
};


/**********************************************************************************************************************/
// Env
/**********************************************************************************************************************/

struct Env {
    // 全局运行标志, 做停止通知用
    volatile bool running = true;

    int port = 0;
    int numUdps = 0;
    std::vector<std::thread> udpReaderThreads;
    std::vector<UdpReader> udpReaders;

    std::thread shakeThread;
    ShakeHandler shakeHandler;

    int numKcpThreads = 0;
    std::vector<std::thread> kcpThreads;
    // 这里使用指针数组而不是 vector 是因为部分结构里有 mutex 导致无法 move
    KcpHandler *kcpHandlers = nullptr;

    Env(int const &port, int const &numUdps, int const &numKcpThreads);

    ~Env();
};




/**********************************************************************************************************************/
// Impls
/**********************************************************************************************************************/

inline int ShakeHandler::FrameUpdate() {
    // 清理超时握手信息
    std::lock_guard<std::mutex> lg(mtx);
    for (auto &&iter = shakes.begin(); iter != shakes.end();) {
        if (iter->second.second < nowMS) {
            iter = shakes.erase(iter);
        } else {
            ++iter;
        }
    }
    return 0;
}

inline uint32_t ShakeHandler::TryAdd(sockaddr_in6 const &addr, int const &timeoutMS) {
    auto &&ip_port = xx::ToString(addr);
    std::lock_guard<std::mutex> lg(mtx);
    auto &&v = shakes[ip_port];
    if (!v.second) {
        v.first = ++convId;
        v.second = nowMS + timeoutMS;
    }
    return v.first;
}

inline void ShakeHandler::HandleShake(int const &fd, sockaddr_in6 const &addr, uint32_t const &serial) {
    // 构造 8 字节回包
    struct SerialConv {
        uint32_t serial;
        uint32_t conv;
    } sc{serial, TryAdd(addr)};
    // 发出
    sendto(fd, &sc, sizeof(sc), 0, (sockaddr *) &addr, sizeof(addr));
}

inline bool ShakeHandler::FindAndRemove(sockaddr_in6 const &addr, uint32_t const &conv) {
    auto &&ip_port = xx::ToString(addr);
    std::lock_guard<std::mutex> lg(mtx);
    auto &&iter = shakes.find(ip_port);
    if (iter == shakes.end()) return false;
    if (iter->second.first == conv) {
        shakes.erase(iter);
        return true;
    }
    return false;
}


inline int KcpHandler::Wait(int const &ms) {
    // 如果有灌入数据就交换队列
    {
        std::lock_guard<std::mutex> lg(mtx);
        if (!fads1.empty()) {
            std::swap(fads1, fads2);
        } else {
            goto LabEnd;
        }
    }
    for (auto &&o : fads2) {
        auto &&conv = *(uint32_t *) o.data.buf;

        // 根据 conv 查找 peer
        auto &&peerIter = cps.find(conv);
        if (peerIter != cps.end()) {
            // 找到就灌入数据
            // 更新地址信息
            memcpy(&peerIter->second->addr, &o.addr, sizeof(o.addr));
            // 将数据灌入 kcp. 进而可能触发 peer->Receive 进而 Close
            peerIter->second->Input(o.data.buf, o.data.len, false);
        } else {
            // 如果不存在 就在 shakes 中找. 如果找不到或 conv 对不上就退出
            if (!env->shakeHandler.FindAndRemove(o.addr, conv)) continue;
            // 创建 peer
            auto &&peer = xx::Make<KcpPeer>(this, o.fd, conv);
            // 放入容器( 这个容器不会加持 )
            cps[conv] = &*peer;
            // 更新地址信息
            memcpy(&peer->addr, &o.addr, sizeof(o.addr));
            // 触发事件回调
            Accept(peer);
            // 如果 已Close 或 未持有 就短路出去
            if (!peer->Alive() || peer.use_count() == 1) continue;
            // 将数据灌入 kcp ( 可能继续触发 Receive 啥的 )
            peer->Input(o.data.buf, o.data.len, true);
        }
    }
    fads2.clear();
    return 0;
    LabEnd:
    //std::this_thread::sleep_for(std::chrono::milliseconds(ms));
    std::this_thread::sleep_for(std::chrono::microseconds(1));
    return 0;
}

inline int KcpHandler::FrameUpdate() {
    // 更新所有 kcps
    for (auto &&kv : cps) {
        kv.second->UpdateKcpLogic();
    }
    return 0;
}

inline void KcpHandler::Accept(std::shared_ptr<KcpPeer> const &peer) {
    peer->Hold();
    peer->SetTimeoutSeconds(10);
}

inline void KcpHandler::Push(FdAddrData &&fad) {
    {
        std::lock_guard<std::mutex> lg(mtx);
        fads1.push_back(std::move(fad));
    }
}

inline UdpReader::UdpReader(Env *const &env)
        : env(env) {
    // 创建端口复用的指定buf尺寸的 udp fd
    fd = xx::Net::MakeSocketFD(env->port, SOCK_DGRAM, nullptr, false, true, 1784 * 3000, 1784 * 3000);
    if (fd < 0)
        throw std::logic_error("create fd failed.");
    xx::CoutN("fd = ", fd);
}

inline UdpReader::~UdpReader() {
    Close();
}

std::atomic<uint64_t> udpRecvCounter = 0;

inline void UdpReader::Run() {
    while (env->running) {
        // 准备数据容器
        FdAddrData fad;
        fad.data.Reserve(2000);
        socklen_t addrLen = sizeof(fad.addr);
        ssize_t len = 0;

        // 读数据到 buf
        do {
            len = recvfrom(fd, fad.data.buf, fad.data.cap, 0, (struct sockaddr *) &fad.addr, &addrLen);
        } while (len == -1 && errno == EINTR);
        if (len == -1) {
            if (!env->running) return;
            throw std::logic_error("recvfrom failed.");
        }
        fad.data.len = len;
        udpRecvCounter++;

        if (len == 4) {
            env->shakeHandler.HandleShake(fd, fad.addr, *(uint32_t *) fad.data.buf);
        } else if (len >= 24) {
            //xx::CoutN("fd = ", fd," fad.addr = ", fad.addr, " fad.data = ", fad.data);
            // 完善信息
            fad.fd = fd;
            fad.conv = *(uint32_t *) fad.data.buf;
            // 根据 conv 选择一个 KcpHandler 投递
            env->kcpHandlers[fad.conv % env->numKcpThreads].Push(std::move(fad));
        }
        // 别的忽略
    }
}

inline void UdpReader::Close() {
    if (fd > 0) {
        close(fd);
        fd = 0;
    }
}

inline UdpReader::UdpReader(UdpReader &&o) noexcept {
    std::swap(env, o.env);
    std::swap(fd, o.fd);
}

inline UdpReader &UdpReader::operator=(UdpReader &&o) noexcept {
    std::swap(env, o.env);
    std::swap(fd, o.fd);
    return *this;
}


inline KcpPeer::KcpPeer(KcpHandler *const &looper, int const &fd, uint32_t const &conv)
        : Item(looper), fd(fd), conv(conv), createMS(xx::NowSteadyEpochMS()) {
    kcp = ikcp_create(conv, this);
    (void) ikcp_wndsize(kcp, 1024, 1024);
    (void) ikcp_nodelay(kcp, 1, 10, 2, 1);
    kcp->rx_minrto = 10;
    kcp->stream = 1;
    ikcp_setoutput(kcp, [](const char *inBuf, int len, ikcpcb *_, void *user) -> int {
        auto self = (KcpPeer *) user;
        return sendto(self->fd, inBuf, len, 0, (sockaddr *) &self->addr, sizeof(self->addr));
    });
}

inline bool KcpPeer::Close(int const &reason, char const *const &desc) {
    if (!kcp) return false;
    ikcp_release(kcp);
    kcp = nullptr;
    GetKcpHandler()->cps.erase(conv);
    if (reason) {
        DelayUnhold();
    }
    return true;
}

inline KcpPeer::~KcpPeer() {
    Close(0, __LINESTR__ "~KcpPeer");
}

inline void KcpPeer::UpdateKcpLogic() {
    // 计算出当前 ms
    // 已知问题: 受 ikcp uint32 限制, 连接最多保持 50 多天
    auto &&currentMS = uint32_t(xx::NowSteadyEpochMS() - createMS);
    // 如果 update 时间没到 就退出
    if (nextUpdateMS > currentMS) return;
    // 来一发
    ikcp_update(kcp, currentMS);
    // 更新下次 update 时间
    nextUpdateMS = ikcp_check(kcp, currentMS);
}

inline void KcpPeer::Input(char const *const &buf, size_t const &len_, bool isFirst) {
    // 将底层数据灌入 kcp
    if (int r = ikcp_input(kcp, buf, len_)) {
        Close(-1, __LINESTR__ "KcpPeer Input if (int r = ikcp_input(kcp, buf, len_))");
        return;
    }
    // 开始处理收到的数据
    do {
        // 如果接收缓存没容量就扩容( 通常发生在首次使用时 )
        if (!recv.cap) {
            recv.Reserve(1024 * 256);
        }
        // 如果数据长度 == buf限长 就自杀( 未处理数据累计太多? )
        if (recv.len == recv.cap) {
            Close(-2, __LINESTR__ "KcpPeer Input if (recv.len == recv.cap)");
            return;
        }

        // 从 kcp 提取数据. 追加填充到 recv 后面区域. 返回填充长度. <= 0 则下次再说
        auto &&len = ikcp_recv(kcp, recv.buf + recv.len, (int) (recv.cap - recv.len));
        if (len <= 0) break;
        recv.len += len;

        // 如果是 5 字节 accept 首包，则忽略
        if (isFirst && recv.len >= 5 && *(uint32_t *) recv.buf == 1 && recv.buf[4] == 0) {
            recv.RemoveFront(5);
            if (!recv.len) continue;
        }

        // 调用用户数据处理函数
        Receive();

        // 如果用户那边已 Close 就直接退出
        if (!Alive()) return;
    } while (true);
}

inline int KcpPeer::Send(char const *const &buf, size_t const &len) {
    if (!kcp) return -1;
    return ikcp_send(kcp, buf, len);
}

inline int KcpPeer::Flush() {
    if (!kcp) return -1;
    ikcp_flush(kcp);
    return 0;
}

inline bool KcpPeer::Alive() const {
    return kcp != nullptr;
}

inline void KcpPeer::Receive() {
    // echo back
    Send(recv.buf, recv.len);
    Flush();
    recv.Clear();
    // 续命
    SetTimeoutSeconds(10);
}

inline Env::Env(int const &port, int const &numUdps, int const &numKcpThreads)
        : port(port), numUdps(numUdps), numKcpThreads(numKcpThreads) {
    // init udpReaders
    for (int i = 0; i < numUdps; ++i) {
        udpReaders.emplace_back(this);
    }
    // init udpReaderThreads
    for (int i = 0; i < numUdps; ++i) {
        udpReaderThreads.emplace_back([p = &udpReaders[i]] {
            p->Run();
        });
    }

    // init shakeHandler & shakeThread
    shakeHandler.env = this;
    shakeHandler.SetFrameRate(1);
    shakeThread = std::thread([&] {
        shakeHandler.Run();
    });

    // init kcpHandlers & kcpThreads
    kcpHandlers = new KcpHandler[numKcpThreads]();
    for (int i = 0; i < numKcpThreads; ++i) {
        auto &&o = kcpHandlers[i];
        o.env = this;
        o.id = i;
        o.SetFrameRate(100);
        kcpThreads.emplace_back([p = kcpHandlers + i] {
            p->Run();
        });
    }
}

inline Env::~Env() {
    // 各种通知
    running = false;
    shakeHandler.running = false;
    for (int i = 0; i < numKcpThreads; ++i) {
        kcpHandlers[i].running = false;
    }
    for (auto &&o : udpReaders) {
        o.Close();
    }

    // 等线程退出
    for (auto &&t : udpReaderThreads) {
        t.join();
    }
    shakeThread.join();
    for (auto &&t : kcpThreads) {
        t.join();
    }

    // 手动回收指针数组
    delete[] kcpHandlers;
    kcpHandlers = nullptr;
}





/**********************************************************************************************************************/
// main
/**********************************************************************************************************************/

int main(int argc, char const *argv[]) {
    if (argc < 4) {
        throw std::logic_error("need 3 args: port  numUdps  numKcpThreads");
    }
    int port = 0, numUdps = 0, numKcpThreads = 0;
    xx::Convert(argv[1], port);
    xx::Convert(argv[2], numUdps);
    xx::Convert(argv[3], numKcpThreads);

    Env env(port, numUdps, numKcpThreads);

    while (true) {
        std::cout << udpRecvCounter << std::endl;
        udpRecvCounter = 0;
        std::this_thread::sleep_for(std::chrono::seconds(1));
    }

    std::cin.get();
    return 0;
}























































//#if !defined(NDEBUG)
//#define BOOST_MULTI_INDEX_ENABLE_INVARIANT_CHECKING
//#define BOOST_MULTI_INDEX_ENABLE_SAFE_MODE
//#endif
//
//#include <boost/multi_index_container.hpp>
//#include <boost/multi_index/member.hpp>
//#include <boost/multi_index/ordered_index.hpp>
//#include <iostream>
//#include <chrono>
//#include <vector>
//#include <unordered_map>
//#include "tsl/hopscotch_map.h"
//
//int64_t NowMS() {
//    return std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch()).count();
//};
//
//struct Player {
//    int64_t id;
//    int64_t money;
//};
//struct Log {
//    int64_t playerId;
//    int64_t money;
//};
//
////struct Player_id {
////};
////using Players = boost::multi_index_container<Player,
////    boost::multi_index::indexed_by<
////    boost::multi_index::ordered_unique<boost::multi_index::tag<Player_id>, ::boost::multi_index::member<Player, int64_t, &Player::id> >
////    >
////>;
////
////struct Log_playerId {
////};
////struct Log_money {
////};
////struct Log_time {
////};
////using Logs = boost::multi_index_container<Log,
////    boost::multi_index::indexed_by<
////    boost::multi_index::ordered_non_unique<boost::multi_index::tag<Log_playerId>, ::boost::multi_index::member<Log, int64_t, &Log::playerId> >,
////    boost::multi_index::ordered_non_unique<boost::multi_index::tag<Log_money>, ::boost::multi_index::member<Log, int64_t, &Log::money> >
////    >
////>;
//int main() {
//    //Players players;
//    //Logs logs;
//    //for (int i = 0; i < 100; ++i) {
//    //    players.emplace(Player{ i, i });
//    //}
//    //for (int j = 0; j < 10000; ++j) {
//    //    for (int i = 0; i < 100; ++i) {
//    //        logs.emplace(Log{ i, j, i });
//    //    }
//    //}
//    //auto&& ms = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch()).count();
//    //for (auto&& p : players) {
//    //    auto&& q = logs.equal_range(p.id);
//    //    for (auto iter = q.first; iter != q.second; ++iter) {
//    //        (int64_t&)p.money += iter->money;
//    //    }
//    //}
//    //std::cout << std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch()).count() - ms << std::endl;
//    //for (auto&& p : players) {
//    //    std::cout << p.id << ", " << p.money << std::endl;
//    //}
//    auto&& ms = NowMS();
//    std::vector<Player> players;
//    players.reserve(10000);
//    std::vector<Log> logs;
//    logs.reserve(10000);
//    for (int64_t i = 0; i < 10000; ++i) {
//        auto&& o = players.emplace_back();
//        o.id = o.money = i;
//    }
//    for (int64_t j = 0; j < 10000; ++j) {
//        for (int64_t i = 0; i < 10000; ++i) {
//            auto&& o = logs.emplace_back();
//            o.playerId = i;
//            o.money = j;
//        }
//    }
//    std::cout << NowMS() - ms << std::endl;
//    ms = NowMS();
//    tsl::hopscotch_map<int64_t, int64_t> ii;
//    for (auto&& L : logs) {
//        ii[L.playerId] += L.money;
//    }
//    for (auto&& p : players) {
//        p.money += ii[p.id];
//    }
//    std::cout << NowMS() - ms << std::endl;
//    for (int k = 0; k < 10; ++k) {
//        std::cout << players[k].id << ", " << players[k].money << std::endl;
//    }
//    return 0;
//}
