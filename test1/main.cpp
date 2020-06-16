#include "xx_epoll.h"
#include "ikcp.h"

namespace xx::Epoll {
    struct ContextEx;
    /***********************************************************************************************************/
    // 基础 udp 组件
    struct UdpPeer : Timer {
        // 对方的 addr( udp 收到数据时会填充. SendTo 时用到 )
        sockaddr_in6 addr;

        // 继承构造函数
        using Timer::Timer;
        // 有数据需要接收( 不累积, 直接投递过来 )
        virtual void Receive(char const* const& buf, size_t const& len) = 0;
        // 直接发送( 如果 fd == -1 则忽略 ), 返回已发送字节数. -1 为出错
        ssize_t Send(char const* const& buf, size_t const& len);
        // 判断 fd 的有效性
        inline bool Alive() { return fd != -1; }
    protected:
        friend Context;
        // epoll 事件处理
        void EpollEvent(uint32_t const &e) override;
    };

    //template<typename KcpPeer>
    struct KcpPeer;
    struct KcpListener : UdpPeer {
        // 自增生成连接id
        uint32_t convId = 0;
        // kcp conv 值与 peer 的映射。KcpPeer Close 时从该字典移除 key
        std::unordered_map<uint32_t, std::shared_ptr<KcpPeer>> cps;
        // 带超时的握手信息字典 key: ip:port   value: conv, nowMS
        std::unordered_map<std::string, std::pair<uint32_t, int64_t>> shakes;

        // 构造函数( 启动 timer 用作每帧驱动 update )
        KcpListener(std::shared_ptr<Context> const& ec, int const& fd);
        // 连接创建成功后会触发
        virtual void OnAccept(std::shared_ptr<KcpPeer> const& peer) = 0;
    protected:
        friend Context;
        friend ContextEx;
        // 每帧 call cps Update, 清理超时握手数据
        void Timeout() override;
        // 关闭 fd, Close & 清除 cps, 延迟减持
        bool Close(int const& reason) override;
        // 判断收到的数据内容, 模拟握手， 最后产生能 KcpPeer
        void Receive(char const* const& buf, size_t const& len) override;
    };

    struct KcpPeer : Timer {
        // 用于收发数据的物理 udp peer( 后面根据标识来硬转为 Listener 或 Dialer )
        std::shared_ptr<UdpPeer> owner;
        // 对方的 addr( owner 收到数据时会填充. SendTo 时用到 )
        sockaddr_in6 addr;

        // kcp 相关上下文
        ikcpcb* kcp = nullptr;
        uint32_t conv = 0;
        int64_t createMS = 0;
        uint32_t nextUpdateMS = 0;

        // 初始化 kcp 相关上下文
        explicit KcpPeer(std::shared_ptr<KcpListener> const& listener);
        //explicit KcpPeer(std::shared_ptr<KcpDialer> const& listener);
        // 回收 kcp 相关上下文
        ~KcpPeer() override;

        // 被 ep 调用. 受帧循环驱动. 帧率越高, kcp 工作效果越好. 典型的频率为 100 fps
        void UpdateKcpLogic();

        // 被 owner 调用. 塞数据到 kcp
        void Input(char* const& buf, size_t const& len);

        // 回收 kcp 对象, 看情况从 ep->kcps 移除
        bool Close(int const& reason) override;

        void Timeout() override;

        virtual int Send(char const* const& buf, size_t const& len);
        virtual int Flush();
        virtual void Receive(char const* const& buf, size_t const& len) {}  // = 0;
    };

    struct ContextEx : Context {
        std::array<char, 64 * 1024> udpBuf;
    };

    inline KcpPeer::KcpPeer(std::shared_ptr<KcpListener> const& listener) : Timer(listener->ec, -1) {
        assert(!kcp);
        // 创建并设置 kcp 的一些参数
        kcp = ikcp_create(conv, this);
        (void)ikcp_wndsize(kcp, 1024, 1024);
        (void)ikcp_nodelay(kcp, 1, 10, 2, 1);
        kcp->rx_minrto = 10;
        kcp->stream = 1;

        // 给 kcp 绑定 output 功能函数
        ikcp_setoutput(kcp, [](const char* inBuf, int len, ikcpcb* kcp, void* user)->int {
            auto self = (KcpPeer*)user;
            // 如果物理 peer 没断, 就通过修改 addr 的方式告知 Send 函数目的地
            if (self->owner) {
                self->owner->addr = self->addr;
                return self->owner->Send(inBuf, len);
            }
            return -1;
        });

        conv = listener->convId;
        createMS = ec->nowMS;
    }

    inline KcpPeer::~KcpPeer() {
        Close(0);
    }

    inline void KcpPeer::Timeout() {
        Close(__LINE__);
    }

    inline int KcpPeer::Send(char const* const& buf, size_t const& len) {

    }

    inline int KcpPeer::Flush() {

    }

    inline void KcpPeer::UpdateKcpLogic() {
        assert(kcp);
        // 计算出当前 ms
        // 已知问题: 受 ikcp uint32 限制, 连接最多保持 50 多天
        auto&& currentMS = uint32_t(ec->nowMS - createMS);
        // 如果 update 时间没到 就退出
        if (nextUpdateMS > currentMS) return;
        // 来一发
        ikcp_update(kcp, currentMS);
        // 更新下次 update 时间
        nextUpdateMS = ikcp_check(kcp, currentMS);
    }

    inline void KcpPeer::Input(char* const& buf, size_t const& len) {
        if (ikcp_input(kcp, buf, len)) {
            Dispose();
            return;
        }

        // 开始处理收到的数据
        do {
            // 如果接收缓存没容量就扩容( 通常发生在首次使用时 )
            if (!recv.cap) {
                recv.Reserve(readBufLen);
            }
            // 如果数据长度 == buf限长 就自杀( 未处理数据累计太多? )
            if (recv.len == recv.cap) {
                Ref<KcpPeer> alive(this);
                OnDisconnect(-1);
                if (alive) {
                    Dispose();
                }
                return;
            }

            // 从 kcp 提取数据. 追加填充到 recv 后面区域. 返回填充长度. <= 0 则下次再说
            auto&& len = ikcp_recv(kcp, recv.buf + recv.len, (int)(recv.cap - recv.len));
            if (len <= 0) break;
            recv.len += len;

            // 调用用户数据处理函数
            {
                Ref<KcpPeer> alive(this);
                OnReceive();
                if (!alive) return;
            }
        } while (true);
    }

    inline bool KcpPeer::Close(int const& reason) {
        if (!kcp) return false;
        // todo: 根据物理 peer Alive 来判断要不要从中移除( 分别针对 Listener 和 Dialer )
        // 回收 kcp 相关
        ikcp_release(kcp);
        kcp = nullptr;
        // 看看 owner 是不是 conn 连接对象。如果是 则 kcp peer 死亡时必须回收它. 同时从 ep->kcps 移除
        // 如果是 KcpListener, 则从 listener->kcps 根据 conv Remove( this )
        if (owner->Alive()) {
            if (dynamic_cast<KcpConn*>(o)) {
                owner->Close(__LINE__);
                assert(indexAtKcps != -1);
                XX_SWAP_REMOVE(this, indexAtKcps, ep->kcps);
            }
            else {
                auto listener = dynamic_cast<KcpListener*>(o);
                assert(listener);
                assert(conv);
                listener->kcps.Remove(conv);
            }
        }

        DelayUnhold();
        return true;
    }



    inline KcpListener::KcpListener(std::shared_ptr<Context> const& ec, int const& fd) : UdpPeer(ec, fd) {
        SetTimeout(1);
    }

    inline void KcpListener::Receive(char const* const& buf, size_t const& len) {
        // addr 转为 ip:port 当 key( 似乎性能有点费? )
        auto ip_port = AddressToString(addr);

        // 当前握手方案为 UdpDialer 每秒 N 次不停发送 4 字节数据( serial )过来,
        // 收到后根据其 ip:port 做 key, 生成 convId. 每次收到都向其发送 convId
        if (len == 4) {
            auto&& iter = shakes.find(ip_port);
            if (iter == shakes.end()) {
                shakes.emplace(ip_port, std::make_pair(++convId, ec->nowMS + 3000));
            }
            // 回发 serial + convId
            char tmp[8];
            memcpy(tmp, buf, 4);
            memcpy(tmp + 4, &convId, 4);
            Send(tmp, 8);
            return;
        }

        // 忽略长度小于 kcp 头的数据包 ( IKCP_OVERHEAD at ikcp.c )
        if (len < 24) return;

        // read conv header
        uint32_t conv;
        memcpy(&conv, buf, sizeof(conv));

        // 准备创建或找回 KcpPeer
        KcpPeer* p = nullptr;

        // 根据 conv 试定位到 peer
        auto&& peerIter = cps.find(conv);

        // 如果不存在 就在 shakes 中按 ip:port 找
        if (peerIter == cps.end()) {
            auto&& iter = shakes.find(ip_port);
            // 未找到或 conv 对不上: 忽略
            if (iter == shakes.end() || iter->second.first != conv) return;

            // 从握手信息移除
            shakes.erase(iter);

            // 始创建 peer // todo: 后面改模板类型
            auto&& peer = xx::Make<KcpPeer>(xx::As<KcpListener>(shared_from_this()));

//            // 继续初始化
//            peer->ep = ep;
//            peer->owner = this;
//            peer->conv = conv;
//            peer->createMS = ep->nowMS;
//
//            // 如果初始化 kcp 失败就忽略
//            if (peer->InitKcp()) return;
//
//            // 更新地址信息
//            memcpy(&peer->addr, &addr, sizeof(addr));
//
//            // 放入容器
//            p = ep->AddItem(std::move(peer));
//            kcps.Add(conv, p);
//
//            // 初始化
//            Ref<KcpPeer> alive(p);
//            p->Init();
//            if (!alive) return;
//
//            // 触发事件回调
//            Accept(p);
//            if (!alive) return;
//        }
//        else {
//            // 定位到目标 peer
//            p = kcps.ValueAt(peerIter);
//
//            // 更新地址信息
//            memcpy(&p->addr, &addr, sizeof(addr));
        }
//
//        // 将数据灌入 kcp. 进而可能触发 peer->OnReceive 进而 Dispose
//        p->Input(recv.buf, (uint32_t)recv.len);
    }

    inline void KcpListener::Timeout() {
        // 更新所有 kcps
        for (auto&& kv : cps) {
            //kv.second->Update();
        }
        // 清理超时握手信息
        for(auto&& iter = shakes.begin(); iter != shakes.end(); ) {
            if (iter->second.second < ec->nowMS) {
                iter = shakes.erase(iter);
            }
            else {
                ++iter;
            }
        }
        // 下帧继续触发
        SetTimeout(1);
    }

    inline bool KcpListener::Close(int const& reason) {
        if (!this->Item::Close(reason)) return false;
        // peer 能检测到如果 listener !Alive() 则不再从 cps 移除自己
        for (auto&& kv : cps) {
            //kv.second->Close(__LINE__);
        }
        cps.clear();
        DelayUnhold();
        return true;
    }

    inline ssize_t UdpPeer::Send(char const* const& buf, size_t const& len) {
        return sendto(fd, buf, len, 0, (sockaddr*)&addr, sizeof(addr));
    }

    inline void UdpPeer::EpollEvent(const uint32_t &e) {
        // error
        if (e & EPOLLERR || e & EPOLLHUP) {
            Close(__LINE__);
            return;
        }
        // read
        if (e & EPOLLIN) {
            auto&& buf = ((ContextEx*)&*ec)->udpBuf;
            socklen_t addrLen = sizeof(addr);
            auto len = recvfrom(fd, buf.data(), buf.size(), 0, (struct sockaddr*) & addr, &addrLen);
            if (len < 0) {
                Close(__LINE__);
                return;
            }
            if (!len) return;
            Receive(buf.data(), len);
        }
    }
}

int main() {
    return 0;
}















//#include "xx_sqlite.h"
//#include "xx_chrono.h"
//#include <iostream>
//
//namespace XS = xx::SQLite;
//
//int main() {
//    //XS::Connection db(":memory:");
//    XS::Connection db("asdf.db3");
//    if (!db) return -1;
//    try {
//        if (!db.TableExists("log")) {
//            db.Execute(R"=-=(
//CREATE TABLE [log](
//    [id] INTEGER PRIMARY KEY AUTOINCREMENT NOT NULL UNIQUE,
//    [time] INTEGER NOT NULL,
//    [desc] TEXT NOT NULL
//);)=-=");
//        } else {
//            db.TruncateTable("log");
//        }
//        XS::Query insertQuery(db, "insert into log (`id`, `time`, `desc`) values (?, ?, ?)");
//        auto ms1 = xx::NowSteadyEpochMS();
//        for (int j = 0; j < 10; ++j) {
//            db.BeginTransaction();
//            for (int i = 0; i < 1000; ++i) {
//                insertQuery.SetParameters(j * 1000 + i, xx::NowEpoch10m(), std::to_string(j * 1000 + i) + " asdfasdf");
//                insertQuery.Execute();
//            }
//            db.Commit();
//        }
//        std::cout << "ms = " << (xx::NowSteadyEpochMS() - ms1) << std::endl;
//
//        //XS::Query selectQuery(db, "select `id`, `time`, `desc` from log");
////        selectQuery.Execute([](XS::Reader &r) {
////            auto id = r.ReadInt32(0);
////            auto time = r.ReadInt64(1);
////            auto desc = r.ReadString(2);
////            std::cout << id << ", " << time << ", " << desc << std::endl;
////        });
//    }
//    catch (const std::exception &e) {
//        std::cout << "catch exception: " << e.what() << std::endl;
//    }
//    return 0;
//}



//#include <xx_datareader.h>
//#include <iostream>
//#include <cassert>
//
//int main() {
//	xx::Data data;
//
//	xx::Write(data, 1, 2, 3, 4, 5, "asdf");
//
//	int a, b, c, d, e;
//	std::string s;
//	int r = xx::Read(data, a, b, c, d, e, s);
//	assert(r == 0);
//	assert(a == 1 && b == 2 && c == 3 && d == 4 && e == 5 && s == "asdf");
//
//	xx::DataReader dr(data);
//	r = dr.Read(a, b, c, d, e);
//	assert(r == 0);
//	auto offset = dr.offset;
//	r = dr.ReadLimit<3>(s);
//	assert(r != 0);
//	dr.offset = offset;
//    r = dr.ReadLimit<4>(s);
//    assert(r == 0);
//
//	std::vector<std::string> ss;
//	ss.emplace_back("asdf");
//	ss.emplace_back("qwer");
//	ss.emplace_back("zxcvb");
//	data.Clear();
//	xx::Write(data, ss);
//	r = xx::Read(data, ss);
//	assert(r == 0);
//
//	dr.Reset(data);
//	r = dr.ReadLimit<3, 5>(ss);
//	assert(r == 0);
//
//	dr.Reset(data);
//	r = dr.ReadLimit<3, 4>(ss);
//	assert(r != 0);
//
//	std::optional<std::vector<std::optional<std::string>>> ss2;
//	ss2.emplace();
//	ss2->emplace_back("asdf");
//	ss2->emplace_back();
//	ss2->emplace_back("qwer");
//	data.Clear();
//	xx::Write(data, ss2);
//
//	return 0;
//}
