#pragma once
#include "xx_epoll.h"
#include "ikcp.h"

namespace xx::Epoll {
    /***********************************************************************************************************/
    // 基础 udp 组件
    struct UdpPeer : Timer {
        // 对方的 addr( udp 收到数据时会填充. SendTo 时用到 )
        sockaddr_in6 addr{};

        // 继承构造函数
        using Timer::Timer;
        // 有数据需要接收( 不累积, 直接投递过来 )
        virtual void Receive(char const *const &buf, size_t const &len) = 0;
        // 直接发送( 如果 fd == -1 则忽略 ), 返回已发送字节数. -1 为出错
        ssize_t Send(char const *const &buf, size_t const &len);
        // 判断 fd 的有效性
        inline bool Alive() { return fd != -1; }
    protected:
        friend Context;
        // epoll 事件处理
        void EpollEvent(uint32_t const &e) override;
    };

    struct KcpPeer;
    struct KcpBase : UdpPeer {

        // 自增生成连接id
        uint32_t convId = 0;
        // kcp conv 值与 peer 的映射。KcpPeer Close 时从该字典移除 key
        std::unordered_map<uint32_t, KcpPeer*> cps;
        // 带超时的握手信息字典 key: ip:port   value: conv, nowMS
        std::unordered_map<std::string, std::pair<uint32_t, int64_t>> shakes;

        // 构造函数( 启动 timer 用作每帧驱动 update )
        explicit KcpBase(std::shared_ptr<Context> const &ec);
        // 方便派生类覆盖 Close 函数:
        // if ... return false;
        // CloseChilds(reason);
        // 从容器变量移除
        // DelayUnhold();
        // return true;
        void CloseChilds(int const &reason);
    protected:
        friend Context;
        // 每帧 call cps Update, 清理超时握手数据
        void Timeout() override;
    };

    struct KcpPeer : Timer {
        // 用于收发数据的物理 udp peer( 后面根据标识来硬转为 Listener 或 Dialer )
        std::shared_ptr<KcpBase> owner;
        // 对方的 addr( owner 收到数据时会填充. SendTo 时用到 )
        sockaddr_in6 addr;
        // 收数据用堆积容器( Receive 里访问它来处理收到的数据 )
        Data recv;

        // kcp 相关上下文
        ikcpcb *kcp = nullptr;
        uint32_t conv = 0;
        int64_t createMS = 0;
        uint32_t nextUpdateMS = 0;

        // 初始化 kcp 相关上下文
        KcpPeer(std::shared_ptr<KcpBase> const &owner, uint32_t const &conv);
        // 回收 kcp 相关上下文
        ~KcpPeer() override;
        // 被 ep 调用. 受帧循环驱动. 帧率越高, kcp 工作效果越好. 典型的频率为 100 fps
        void UpdateKcpLogic();
        // 被 owner 调用. 塞数据到 kcp
        void Input(char const *const &buf, size_t const &len, bool isFirst = false);
        // 回收 kcp 对象, 看情况从 ep->kcps 移除
        bool Close(int const &reason) override;
        // Close
        void Timeout() override;
        // 传进发送队列( 如果 !kcp 则忽略 )
        virtual int Send(char const *const &buf, size_t const &len);
        // 立刻开始发送
        virtual int Flush();
        // 数据接收事件: 用户可以从 recv 拿数据, 并移除掉已处理的部分
        virtual void Receive() = 0;
        // kcp != null && !owner
        bool Alive();
    };

    template<typename PeerType, class ENABLED = std::is_base_of<KcpPeer, PeerType>>
    struct KcpListener : KcpBase {
        using KcpBase::KcpBase;
        // 1. 判断收到的数据内容, 模拟握手， 最后产生 KcpPeer
        // 2. 定位到 KcpPeer, Input 数据
        void Receive(char const *const &buf, size_t const &len) override;
        // 连接创建成功后会触发
        virtual void Accept(std::shared_ptr<PeerType> const &peer) = 0;
        // fd = MakeSocketFD(port, SOCK_DGRAM)
        int Listen(int const& port);
    };


    inline bool KcpPeer::Alive() {
        return kcp != nullptr && owner;
    }

    inline KcpPeer::KcpPeer(std::shared_ptr<KcpBase> const &owner, uint32_t const &conv) : Timer(owner->ec, -1) {
        assert(!kcp);
        // 创建并设置 kcp 的一些参数. 按照 每秒 100 帧来设置的. 即精度 10 ms
        kcp = ikcp_create(conv, this);
        (void) ikcp_wndsize(kcp, 1024, 1024);
        (void) ikcp_nodelay(kcp, 1, 10, 2, 1);
        kcp->rx_minrto = 10;
        kcp->stream = 1;

        // 给 kcp 绑定 output 功能函数
        ikcp_setoutput(kcp, [](const char *inBuf, int len, ikcpcb *_, void *user) -> int {
            auto self = (KcpPeer *) user;
            // 如果物理 peer 没断, 就通过修改 addr 的方式告知 Send 函数目的地
            if (self->owner) {
                self->owner->addr = self->addr;
                return self->owner->Send(inBuf, len);
            }
            return -1;
        });

        // 填充初始参数
        this->owner = owner;
        this->conv = conv;
        this->createMS = ec->nowMS;
    }

    inline KcpPeer::~KcpPeer() {
        Close(0);
    }

    inline void KcpPeer::Timeout() {
        Close(__LINE__);
    }

    inline int KcpPeer::Send(char const *const &buf, size_t const &len) {
        if (!kcp) return -1;
        return ikcp_send(kcp, buf, len);
    }

    inline int KcpPeer::Flush() {
        if (!kcp) return __LINE__;
        ikcp_flush(kcp);
        return 0;
    }

    inline void KcpPeer::UpdateKcpLogic() {
        assert(kcp);
        // 计算出当前 ms
        // 已知问题: 受 ikcp uint32 限制, 连接最多保持 50 多天
        auto &&currentMS = uint32_t(ec->nowMS - createMS);
        // 如果 update 时间没到 就退出
        if (nextUpdateMS > currentMS) return;
        // 来一发
        ikcp_update(kcp, currentMS);
        // 更新下次 update 时间
        nextUpdateMS = ikcp_check(kcp, currentMS);
    }

    inline void KcpPeer::Input(char const *const &buf, size_t const &len_, bool isFirst) {
        // 将底层数据灌入 kcp
        if (ikcp_input(kcp, buf, len_)) {
            Close(__LINE__);
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
                Close(__LINE__);
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

    inline bool KcpPeer::Close(int const &reason) {
        if (!kcp) return false;
        // 回收 kcp
        ikcp_release(kcp);
        kcp = nullptr;
        // 从父容器移除( 由父容器发起的 Close 会导致条件失败: 父容器调用 子Close 时先清 owner )
        if (owner) {
            owner->cps.erase(conv);
        }
        // 如果时析构函数调用 Close 则不需要延迟减持( 已经没了 )
        if (reason) {
            DelayUnhold();
        }
        return true;
    }


    inline KcpBase::KcpBase(std::shared_ptr<Context> const &ec) : UdpPeer(ec, -1) {
        // 注册下一帧帧发起的回调( 回调中继续调用这句代码以实现每帧都产生回调 )
        SetTimeout(1);
    }

    template<typename PeerType, class ENABLED>
    inline int KcpListener<PeerType, ENABLED>::Listen(int const& port) {
        // 先清一把
        Close(0);
        // 创建监听用 socket fd
        auto &&fd = ec->MakeSocketFD(port, SOCK_DGRAM);
        if (fd < 0) return -1;
        // 确保 return 时自动 close
        xx::ScopeGuard sg([&] { close(fd); });
        // fd 纳入 epoll 管理
        if (-1 == ec->Ctl(fd, EPOLLIN)) return -3;
        // 取消自动 close
        sg.Cancel();
        // 补映射( 因为 -1 调用的基类构造, 映射代码被跳过了 )
        this->fd = fd;
        ec->fdMappings[fd] = this;
        return 0;
    }


    template<typename PeerType, class ENABLED>
    inline void KcpListener<PeerType, ENABLED>::Receive(char const *const &buf, size_t const &len) {
        // addr 转为 ip:port 当 key( 似乎性能有点费? )
        auto ip_port = AddressToString(addr);

        // 当前握手方案为 UdpDialer 每秒 N 次不停发送 4 字节数据( serial )过来,
        // 收到后根据其 ip:port 做 key, 生成 convId. 每次收到都向其发送 convId
        if (len == 4) {
            auto &&iter = shakes.find(ip_port);
            if (iter == shakes.end()) {
                shakes.emplace(ip_port, std::make_pair(++convId, ec->nowMS + 3000));
            }
            // 回发 serial + convId。客户端在收到回发数据后，会通过 kcp 发送 01 00 00 00 00 这样的 5 字节数据，以触发服务器 accept
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

        // 根据 conv 试定位到 peer
        auto &&peerIter = cps.find(conv);

        // 找到就灌入数据
        if (peerIter != cps.end()) {
            // 更新地址信息
            memcpy(&peerIter->second->addr, &addr, sizeof(addr));
            // 将数据灌入 kcp. 进而可能触发 peer->Receive 进而 Close
            peerIter->second->Input(buf, len);
        }
        else {
            // 如果不存在 就在 shakes 中按 ip:port 找
            auto &&iter = shakes.find(ip_port);
            // 未找到或 conv 对不上: 忽略
            if (iter == shakes.end() || iter->second.first != conv) return;

            // 从握手信息移除
            shakes.erase(iter);
            // 创建 peer
            auto &&peer = xx::Make<PeerType>(xx::As<KcpBase>(shared_from_this()), conv);
            // 放入容器( 这个容器不会加持 )
            cps[conv] = &*peer;
            // 更新地址信息
            memcpy(&peer->addr, &addr, sizeof(addr));
            // 触发事件回调
            Accept(peer);
            // 如果 已Close 或 未持有 就短路出去
            if (!peer->Alive() || peer.use_count() == 1) return;
            // 将数据灌入 kcp ( 可能继续触发 Receive 啥的 )
            peer->Input(buf, len, true);
        }
    }

    inline void KcpBase::Timeout() {
        // 更新所有 kcps
        for (auto &&kv : cps) {
            kv.second->UpdateKcpLogic();
        }
        // 清理超时握手信息
        for (auto &&iter = shakes.begin(); iter != shakes.end();) {
            if (iter->second.second < ec->nowMS) {
                iter = shakes.erase(iter);
            } else {
                ++iter;
            }
        }
        // 下帧继续触发
        SetTimeout(1);
    }

    inline void KcpBase::CloseChilds(int const &reason) {
        for (auto &&kv : cps) {
            // 先清掉 owner 避免 Close 函数内部到 cps 来移除自己, 同时也是双向减持
            kv.second->owner.reset();
            // 关掉挂靠 peer
            kv.second->Close(__LINE__);
        }
        // 减持所有 挂靠 peer
        cps.clear();
    }

    inline ssize_t UdpPeer::Send(char const *const &buf, size_t const &len) {
        // 保底检查
        if(!Alive()) return -1;
        // 底层直发
        return sendto(fd, buf, len, 0, (sockaddr *) &addr, sizeof(addr));
    }

    inline void UdpPeer::EpollEvent(const uint32_t &e) {
        // error
        if (e & EPOLLERR || e & EPOLLHUP) {
            Close(__LINE__);
            return;
        }
        // read
        if (e & EPOLLIN) {
            socklen_t addrLen = sizeof(addr);
            auto len = recvfrom(fd, ec->buf.data(), ec->buf.size(), 0, (struct sockaddr *) &addr, &addrLen);
            if (len < 0) {
                Close(__LINE__);
                return;
            }
            if (!len) return;
            Receive(ec->buf.data(), len);
        }
    }
}
