#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <netdb.h>
#include <unistd.h>
#include <fcntl.h>
#include <cerrno>
#include <csignal>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/uio.h>
#include <sys/signalfd.h>
#include <sys/inotify.h>
#include <netinet/tcp.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include <memory>
#include <thread>
#include <vector>
#include <mutex>
#include <unordered_map>
#include <tuple>

#include "xx_data.h"
#include "xx_fixeddata.h"
#include "xx_chrono.h"
#include "xx_looper.h"
#include "ikcp.h"
#include "xx_string.h"


// todo:
// 多线程同时监听同一个 udp port, 阻塞收取
// 如果是 kcp 包, 根据 convId 取模，投递到相应处理线程？ 类似 hash ?
// 如果是 握手, 先投递到一个独立线程处理.
// convId 全局线程安全自增
// 每个处理线程跑一个 KcpLooper Run. Wait 时检查自己的队列并处理里面的数据( from fd, from addr, data )




// sockType == SOCK_STREAM || SOCK_DGRAM
inline int MakeSocketFD(int const &port, int const &sockType = SOCK_DGRAM, char const *const &hostName = nullptr,
                        bool const &nonblock = false, bool const &reusePort = true,
                        size_t const &rmem_max = 1784 * 3000, size_t const &wmem_max = 1784 * 3000) {
    char portStr[20];
    snprintf(portStr, sizeof(portStr), "%d", port);

    addrinfo hints;
    memset(&hints, 0, sizeof(addrinfo));
    hints.ai_family = AF_UNSPEC;                                            // ipv4 / 6
    hints.ai_socktype = sockType;                                           // SOCK_STREAM / SOCK_DGRAM
    hints.ai_flags = AI_PASSIVE;                                            // all interfaces

    addrinfo *ai_, *ai;
    if (getaddrinfo(hostName, portStr, &hints, &ai_)) return -1;

    int fd;
    for (ai = ai_; ai != nullptr; ai = ai->ai_next) {
        fd = socket(ai->ai_family, ai->ai_socktype | (nonblock ? SOCK_NONBLOCK : 0), ai->ai_protocol);
        if (fd == -1) continue;

        int enable = 1;
        if (setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, &enable, sizeof(int)) < 0) {
            close(fd);
            continue;
        }
        if (reusePort) {
            if (setsockopt(fd, SOL_SOCKET, SO_REUSEPORT, &enable, sizeof(int)) < 0) {
                close(fd);
                continue;
            }
        }
        if (rmem_max) {
            if (setsockopt(fd, SOL_SOCKET, SO_RCVBUF, (const char *) &rmem_max, sizeof(rmem_max)) < 0) {
                close(fd);
                continue;
            }
        }
        if (wmem_max) {
            if (setsockopt(fd, SOL_SOCKET, SO_SNDBUF, (const char *) &wmem_max, sizeof(wmem_max)) < 0) {
                close(fd);
                continue;
            }
        }
        if (!bind(fd, ai->ai_addr, ai->ai_addrlen)) break;                  // success

        close(fd);
    }
    freeaddrinfo(ai_);
    if (!ai) return -2;
    return fd;
}

namespace xx {
    // 适配 sockaddr_in6
    template<>
    struct StringFuncs<sockaddr_in6, void> {
        static inline void Append(std::string &s, sockaddr_in6 const &in) {
            char hbuf[NI_MAXHOST], sbuf[NI_MAXSERV];
            if (!getnameinfo((sockaddr *) &in,
                             ((sockaddr *) &in)->sa_family == AF_INET6 ? INET6_ADDRSTRLEN : INET_ADDRSTRLEN, hbuf,
                             sizeof hbuf, sbuf, sizeof sbuf, NI_NUMERICHOST | NI_NUMERICSERV)) {
                s.append(hbuf);
                s.push_back(':');
                s.append(sbuf);
            }
        }
    };
}

// udp data
using Data = xx::FixedData<1800>;

// udp from addr, data, from fd
using Addr_Data_FD = std::tuple<sockaddr_in6, Data, int>;
using Addr_Data_FDs = std::vector<Addr_Data_FD>;

// udp from addr, shake, from fd
using Addr_Shake_FD = std::tuple<sockaddr_in6, uint32_t, int>;
using Addr_Shake_FDs = std::vector<Addr_Shake_FD>;

struct Env;

struct UdpReader {
    Env* env = nullptr;
    int fd = 0;

    void Run();
};


struct ShakeLooper : xx::Looper {
    Env *env = nullptr;
protected:
    std::mutex mtx;
    // 带超时的握手信息字典 key: ip:port   value: conv, nowMS. 需要 lock 访问
    std::unordered_map<std::string, std::pair<uint32_t, int64_t>> shakes;
    // 自增生成 conv 用
    uint32_t convId = 0;

    // 扫描超时的逻辑
    int FrameUpdate() override;

    // 根据 ip_port 返回一个 convId. 如果已存在就返回旧的。否则就生成
    uint32_t TryAdd(std::string const &ip_port, int const &timeoutMS = 3000);

public:
    // 4 字节握手包处理函数( 会使用来源 fd 发回 8 字节附带 convId )
    void HandleShake(Addr_Shake_FD const &sdf);

    // 判断 ip_port 是否在握手列表中. 如果在 且 conv 对的上, 就移除并返回 true
    bool FindAndRemove(sockaddr_in6 const &addr, uint32_t const &conv);
};

struct KcpLooper;

struct Env {
    bool running = true;

    std::vector<std::thread> udpThreads;

    std::vector<int> fds;
    std::vector<Addr_Data_FDs> adfss;
    std::unique_ptr<std::mutex[]> mtxs;

    std::vector<std::shared_ptr<KcpLooper>> logicLoopers;
    std::vector<std::thread> logicThreads;

    std::shared_ptr<ShakeLooper> shakeLooper;
    std::thread shakeLooperThread;

    Env(int const &num, int const &port);
};

inline int ShakeLooper::FrameUpdate() {
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

inline uint32_t ShakeLooper::TryAdd(std::string const &ip_port, int const &timeoutMS) {
    std::lock_guard<std::mutex> lg(mtx);
    auto &&v = shakes[ip_port];
    if (!v.second) {
        v.first = ++convId;
        v.second = nowMS + timeoutMS;
    }
    return v.first;
}

inline void ShakeLooper::HandleShake(Addr_Shake_FD const &sdf) {
    auto &&ip_addr = xx::ToString(std::get<0>(sdf));
    struct SerialConv {
        uint32_t serial;
        uint32_t conv;
    } sc{std::get<1>(sdf), TryAdd(ip_addr)};
    sendto(std::get<2>(sdf), &sc, sizeof(sc), 0, (sockaddr *) &std::get<0>(sdf), sizeof(std::get<0>(sdf)));
}

inline bool ShakeLooper::FindAndRemove(sockaddr_in6 const &addr, uint32_t const &conv) {
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


struct KcpPeer;

struct KcpLooper : xx::Looper {
    using xx::Looper::Looper;
    Env *env = nullptr;

    std::mutex mtx;
    Addr_Data_FDs adfs1, adfs2;

    // kcp conv 值与 peer 的映射。KcpPeer Close 时从该字典移除 key
    std::unordered_map<uint32_t, KcpPeer *> cps;

    // kcp update
    int FrameUpdate() override;

    // kcp input from adfs2
    int Wait(int const &ms) override;

    void Accept(std::shared_ptr<KcpPeer> const &peer);
};

struct KcpPeer : xx::Timer {
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
    KcpLooper &GetKcpLooper() {
        return *(KcpLooper *) &*looper;
    }

    // 初始化 kcp 相关上下文
    KcpPeer(std::shared_ptr<KcpLooper> const &looper, int const &fd, uint32_t const &conv)
            : Timer(looper), fd(fd), conv(conv), createMS(xx::NowSteadyEpochMS()) {
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

    // 回收 kcp 并延迟自杀( 非析构 )
    inline bool Close(int const &reason, char const *const &desc) override {
        if (!kcp) return false;
        ikcp_release(kcp);
        kcp = nullptr;
        GetKcpLooper().cps.erase(conv);
        if (reason) {
            DelayUnhold();
        }
        return true;
    }

    // Close(0)
    ~KcpPeer() override {
        Close(0, __LINESTR__ "~KcpPeer");
    }

    // 被 ep 调用. 受帧循环驱动. 帧率越高, kcp 工作效果越好. 典型的频率为 100 fps
    inline void UpdateKcpLogic() {
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

    // 被 looper 调用. 塞数据到 kcp
    inline void Input(char const *const &buf, size_t const &len_, bool isFirst = false) {
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

    // 传进发送队列( 如果 !kcp 则忽略 )
    inline int Send(char const *const &buf, size_t const &len) {
        if (!kcp) return -1;
        return ikcp_send(kcp, buf, len);
    }

    // 立刻开始发送
    inline int Flush() {
        if (!kcp) return -1;
        ikcp_flush(kcp);
        return 0;
    }

    // kcp != null
    inline bool Alive() const {
        return kcp != nullptr;
    }

    // 接收数据处理
    inline void Receive() {
        // echo back
        Send(recv.buf, recv.len);
        Flush();
        recv.Clear();
        // 续命
        SetTimeoutSeconds(10);
    }
};

inline int KcpLooper::Wait(int const &ms) {
    // 如果有灌入数据就交换队列
    {
        std::lock_guard<std::mutex> lg(mtx);
        if (!adfs1.empty()) {
            std::swap(adfs1, adfs2);
        }
    }
    for (auto &&adfs : adfs2) {
        auto &&addr = std::get<0>(adfs);
        auto &&data = std::get<1>(adfs);
        auto &&fd = std::get<2>(adfs);
        auto &&conv = *(uint32_t *) data.buf;

        // 根据 conv 查找 peer
        auto &&peerIter = cps.find(conv);
        if (peerIter != cps.end()) {
            // 找到就灌入数据
            // 更新地址信息
            memcpy(&peerIter->second->addr, &addr, sizeof(addr));
            // 将数据灌入 kcp. 进而可能触发 peer->Receive 进而 Close
            peerIter->second->Input(data.buf, data.len, false);
        } else {
            // 如果不存在 就在 shakes 中找. 如果找不到或 conv 对不上就退出
            if (!env->shakeLooper->FindAndRemove(addr, conv)) continue;
            // 创建 peer
            auto &&peer = xx::Make<KcpPeer>(xx::As<KcpLooper>(shared_from_this()), fd, conv);
            // 放入容器( 这个容器不会加持 )
            cps[conv] = &*peer;
            // 更新地址信息
            memcpy(&peer->addr, &addr, sizeof(addr));
            // 触发事件回调
            Accept(peer);
            // 如果 已Close 或 未持有 就短路出去
            if (!peer->Alive() || peer.use_count() == 1) continue;
            // 将数据灌入 kcp ( 可能继续触发 Receive 啥的 )
            peer->Input(data.buf, data.len, true);
        }
    }
    adfs2.clear();
    return 0;
}

inline int KcpLooper::FrameUpdate() {
    // 更新所有 kcps
    for (auto &&kv : cps) {
        kv.second->UpdateKcpLogic();
    }
    return 0;
}

void KcpLooper::Accept(std::shared_ptr<KcpPeer> const &peer) {
    peer->Hold();
    peer->SetTimeoutSeconds(10);
}

inline void UdpReader::Run() {
    Addr_Shake_FD asf;
    std::get<2>(asf) = fd;
    Addr_Data_FD adf;
    std::get<2>(adf) = fd;
    auto&& addr = std::get<0>(adf);
    auto&& data = std::get<1>(adf);
    socklen_t addrLen = sizeof(addr);
    ssize_t len = 0;
    while (env->running) {
        do {
            len = recvfrom(fd, data.buf, data.cap, 0, (struct sockaddr *) &addr, &addrLen);
        } while (len == -1 && errno == EINTR);
        if (len == -1) throw std::logic_error("recvfrom failed.");

        if (len == 4) {
            memcpy(&std::get<0>(asf), &addr, sizeof(addr));
            env->shakeLooper->HandleShake(asf);
        }
        else if (len >= 24) {
            auto&& conv = *(uint32_t*)data.buf;

        }
        // 别的忽略
    }
}


inline Env::Env(int const &num, int const &port) {
    shakeLooperThread = std::thread([&] {
        xx::MakeTo(shakeLooper);
        shakeLooper->env = this;
        shakeLooper->SetFrameRate(1);
        shakeLooper->Run();
    });

    // 创建所有 fd
    fds.resize(num);
    for (int i = 0; i < num; ++i) {
        auto fd = MakeSocketFD(port);
        if (fd < 0)
            throw std::logic_error("create fd failed.");
        fds[i] = fd;
    }

//    // 创建所有 mutex for 访问 logicThread??
//    mtxs = std::make_unique<std::mutex[]>(num);
//    adfss.resize(num);

    udpThreads.reserve(num);
    // todo

    logicThreads.reserve(num);
    // todo

    for (int i = 0; i < num; ++i) {
        //logicLoopers

    }

    // todo: more init here
}


int main() {
    Env env(5, 5678);
    std::cin.get();
    return 0;
}

//    int num = 5;
//    int port = 5678;
//    bool running = true;
//
//
//    for (int i = 0; i < num; ++i) {
//        threads.emplace_back(std::thread([&, i = i] {
//            auto fd = fds[i];
//            while (running) {
//                Addr_Data ad;
//                ad.data.Reserve(60000);
//                ssize_t len = 0;
//                do {
//                    len = recvfrom(fd, ad.data.buf, ad.data.cap, 0, (struct sockaddr *) &ad.addr, &ad.addrLen);
//                } while (len == -1 && errno == EINTR);
//                if (len == -1) throw std::logic_error("recvfrom failed.");
//                std::lock_guard<std::mutex> lg(mtxs[i]);
//                adss[i].emplace_back(std::move(ad));
//            }
//        }));
//        threads2.emplace_back(std::thread([&, i = i] {
//            std::vector<Addr_Data> ads;
//            while (running) {
//                // 如果有灌入数据就交换队列
//                {
//                    std::lock_guard<std::mutex> lg(mtxs[i]);
//                    if (!adss[i].empty()) {
//                        std::swap(ads, adss[i]);
//                    }
//                    // else goto kcp update
//                }
//                // 灌入 kcp
//                for (auto &&ad : ads) {
//
//                }
//
//                std::this_thread::sleep_for(std::chrono::milliseconds(100));
//            }
//        }));
//    }














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
//#include "ska.h"
//#include "tsl/hopscotch_map.h"
//#include "tsl/bhopscotch_map.h"
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
//    //std::unordered_map<int64_t, int64_t> ii;
//    //ska::flat_hash_map<int64_t, int64_t> ii;
//    //tsl::hopscotch_map<int64_t, int64_t> ii;
//    tsl::bhopscotch_map<int64_t, int64_t> ii;
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
