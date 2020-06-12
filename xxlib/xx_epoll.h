#pragma once

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
#include <sys/epoll.h>
#include <sys/uio.h>
#include <sys/signalfd.h>
#include <sys/inotify.h>
#include <netinet/tcp.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include <termios.h>
#include <readline/readline.h>
#include <readline/history.h>

#include <iostream>
#include <string>
#include <deque>
#include <mutex>

#include "xx_ptr.h"
#include "xx_chrono.h"
#include "xx_data_queue.h"
#include "xx_itempool.h"
#include "xx_scopeguard.h"

#define LIKELY(x)       __builtin_expect(!!(x), 1)
#define UNLIKELY(x)     __builtin_expect(!!(x), 0)

// todo: 统一递增填充各种 return -?


// 注意：

// 所有调用 虚函数 的地方是否有 alive 检测? 需检测上层函数是否已 call Dispose

// 因为 gcc 傻逼，会导致类自杀的类成员函数，一定要复制需要的成员参数到"栈"变量，再调用函数，避免出异常. 且需要逐级扩散排查.
// 考虑在注释位置增加 文字描述 类似  // 重要：可能导致类实例自杀  并逐级扩散以方便检查以及形成调用 & 检测是否已自杀的规范

// 基类析构发起的 Dispose 无法调用 派生类 override 的部分, 需谨慎
// 上层类只析构自己的数据, 到基类析构时无法访问上层类成员与 override 的函数
// 析构过程中无法执行 shared_from_this



namespace xx::Epoll {

    /***********************************************************************************************************/
    // Item
    /***********************************************************************************************************/

    struct Context;

    struct Item {
        // 指向总容器
        Context *ep = nullptr;

        // linux 系统文件描述符. 用不上就保持默认值
        int fd = -1;

        // 留个拿到依赖填充完整后的初始化口子( 比如启动 timer, 发首包啥的 )
        inline virtual void Init() {};

        // epoll 事件处理. 用不上不必实现
        inline virtual void OnEpollEvent(uint32_t const &e) {}

        // item 所在容器下标
        int indexAtContainer = -1;

        // 从 ep->items 移除自己, 进而触发析构
        void Dispose();

        // fd 如果不为 -1, 就解除映射并关闭 fd
        virtual ~Item();
    };

    using Item_u = std::unique_ptr<Item>;


    /***********************************************************************************************************/
    // Ref
    /***********************************************************************************************************/

    // 针对 Item 的 弱引用伪指针. 几个操作符每次都会检查是否失效. 失效会 throw. 可以 try catch。
    // 有别于 shared_ptr + weak_ptr 套装, 这个方案基于 unique_ptr 单例 模式, 降低编写 weak_ptr lock() 的复杂度
    // 强制内存回收，避免对象生命周期延长

    template<typename T>
    struct Ref {
        ItemPool<Item_u> *items = nullptr;
        int index = -1;
        int64_t version = 0;

        // 会从 ptr 中提取 ep & indexAtContainer. 故需要保证这些值有效
        Ref(T *const &ptr);

        Ref(std::unique_ptr<T> const &ptr);

        Ref() = default;

        Ref(Ref const &) = default;

        Ref(Ref &&) noexcept;

        Ref &operator=(Ref const &) = default;

        Ref &operator=(Ref &&) noexcept;

        template<typename U>
        Ref &operator=(std::enable_if_t<std::is_base_of_v<T, U>, Ref<U>> const &o);

        template<typename U>
        Ref &operator=(std::enable_if_t<std::is_base_of_v<T, U>, Ref<U>> &&o);

        template<typename U>
        Ref<U> As() const;

        explicit operator bool() const;

        T *operator->() const;

        [[nodiscard]] T *Lock() const;

        template<typename U = T>
        void Reset(U *const &ptr = nullptr);
    };

    template<typename A, typename B>
    inline bool operator==(Ref<A> const &a, Ref<B> const &b) {
        return a.Lock() == b.Lock();
    }

    template<typename A, typename B>
    inline bool operator!=(Ref<A> const &a, Ref<B> const &b) {
        return a.Lock() != b.Lock();
    }

    using Item_r = Ref<Item>;


    /***********************************************************************************************************/
    // ItemTimeout
    /***********************************************************************************************************/

    // 需要自带超时功能的 item 可继承
    struct ItemTimeout : Item {
        int timeoutIndex = -1;
        ItemTimeout *timeoutPrev = nullptr;
        ItemTimeout *timeoutNext = nullptr;

        int SetTimeout(int const &interval);

        int SetTimeoutSeconds(double const &seconds);

        virtual void OnTimeout() = 0;

        ~ItemTimeout() override;
    };


    /***********************************************************************************************************/
    // Timer
    /***********************************************************************************************************/

    struct Timer;
    using Timer_r = Ref<Timer>;

    struct Timer : ItemTimeout {
        // 时间到达时触发. 如果想实现 repeat 效果, 就在函数返回前 自己 timer->SetTimeout
        std::function<void(Timer_r const &timer)> onFire;

        // 超时触发 onFire + 可选 Dispose
        void OnTimeout() override;
    };


    /***********************************************************************************************************/
    // TcpPeer
    /***********************************************************************************************************/

    struct TcpPeer : ItemTimeout {
        TcpPeer() = default;

        // 对方的 addr
        sockaddr_in6 addr{};

        // 每 fd 每一次可写, 写入的长度限制( 希望能实现当大量数据下发时各个 socket 公平的占用带宽 )
        size_t sendLenPerFrame = 65536;

        // 是否正在发送( 是：不管 sendQueue 空不空，都不能 write, 只能塞 sendQueue )
        bool writing = false;

        // 待发送队列
        xx::DataQueue sendQueue;

        // 收数据用堆积容器
        Data recv;

        // 读缓冲区内存扩容增量
        size_t readBufLen = 65536;

        // 将 fd 移交 给一个新的 TcpPeer 沿用。需确保 没有发送过数据 & 没有剩余数据未处理
        // 常见于 读取到首包之后 才能创建出具体 peer 处理后续数据包. 通常接下来需要 Dispose
        template<typename T>
        Ref<T> CreateByFD();

        // 数据接收事件: 从 recv 拿数据. 默认实现为 echo
        virtual void OnReceive();

        // 断线事件. 默认实现为 空
        virtual void OnDisconnect(int const &reason);

        // buf + len 塞队列并开始发送
        virtual int Send(char const *const &buf, size_t const &len);

        // Data 对象移进队列并开始发送
        virtual int Send(Data &&data);

        // epoll 事件处理
        void OnEpollEvent(uint32_t const &e) override;

    protected:
        int Write();

        // 超时触发 Disconnect(-4) + Dispose
        void OnTimeout() override;
    };

    using TcpPeer_r = Ref<TcpPeer>;
    using TcpPeer_u = std::unique_ptr<TcpPeer>;



    /***********************************************************************************************************/
    // TcpListener
    /***********************************************************************************************************/

    struct TcpListener : Item {
        [[maybe_unused]] typedef TcpPeer PeerType;

        // 提供创建 peer 对象的实现
        virtual TcpPeer_u OnCreatePeer();

        // 提供为 peer 绑定事件的实现
        inline virtual void OnAccept(TcpPeer_r const &peer) {}

        // 调用 accept
        void OnEpollEvent(uint32_t const &e) override;

    protected:
        // return fd. <0: error. 0: empty (EAGAIN / EWOULDBLOCK), > 0: fd
        int Accept(int const &listenFD);
    };



    /***********************************************************************************************************/
    // TcpConn
    /***********************************************************************************************************/

    struct TcpDialer;
    using TcpDialer_r = Ref<TcpDialer>;

    struct TcpConn : Item {
        // 指向拨号器, 方便调用其 OnConnect 函数
        TcpDialer_r dialer;

        // 判断是否连接成功
        void OnEpollEvent(uint32_t const &e) override;
    };

    using TcpConn_r = Ref<TcpConn>;



    /***********************************************************************************************************/
    // TcpDialer
    /***********************************************************************************************************/

    struct TcpDialer : ItemTimeout {
        // 要连的地址数组. 带协议标记
        std::vector<sockaddr_in6> addrs;

        // 向 addrs 追加地址. 如果地址转换错误将返回非 0
        int AddAddress(std::string const &ip, int const &port);

        // 开始拨号。会遍历 addrs 为每个地址创建一个 ?cpConn 连接
        // 保留先连接上的 socket fd, 创建 Peer 并触发 OnConnect 事件.
        // 如果超时，也触发 OnConnect，参数为 nullptr
        int Dial(int const &timeoutFrames);

        int DialSeconds(double const &timeoutSeconds);

        // 返回是否正在拨号
        bool Busy();

        // 停止拨号 并清理 conns. 保留 addrs.
        void Stop();

        // 连接成功或超时后触发
        virtual void OnConnect(TcpPeer_r const &peer) = 0;

        // 覆盖并提供创建 peer 对象的实现. 返回 nullptr 表示创建失败
        virtual TcpPeer_u OnCreatePeer();

        // Stop()
        ~TcpDialer() override;

        // 存个空值备用 以方便返回引用
        TcpPeer_r emptyPeer;

        // 内部连接对象. 拨号完毕后会被清空
        std::vector<Item_r> conns;

        // 超时表明所有连接都没有连上. 触发 OnConnect( nullptr )
        void OnTimeout() override;

    protected:
        // 按具体协议创建 Conn 对象
        int NewTcpConn(sockaddr_in6 const &addr);
    };


    /***********************************************************************************************************/
    // CommandHandler
    /***********************************************************************************************************/

    // 处理键盘输入指令的专用类( 单例 ). 直接映射到 STDIN_FILENO ( fd == 0 )
    struct CommandHandler : Item {
        inline static CommandHandler *self = nullptr;
        std::vector<std::string> args;

        CommandHandler();

        static void ReadLineCallback(char *line);

        static char **CompleteCallback(const char *text, int start, int end);

        static char *CompleteGenerate(const char *text, int state);

        void OnEpollEvent(uint32_t const &e) override;

        ~CommandHandler() override;

    protected:
        // 解析 row 内容并调用 cmd 绑定 handler
        void Exec(char const *const &row, size_t const &len);
    };


    /***********************************************************************************************************/
    // PipeReader
    /***********************************************************************************************************/

    // 用于读取 pipe( 这里只是用于跨线程 dispatch 通知, 不是真的读 )
    struct PipeReader : Item {
        void OnEpollEvent(uint32_t const &e) override;
    };


    /***********************************************************************************************************/
    // Context
    /***********************************************************************************************************/

    struct Context {
        /********************************************************/
        // 下面这几个是内部成员, 别碰

        // 所有类实例唯一容器。外界用 Ref 来存引用. 自带自增版本号管理
        ItemPool<Item_u> items;

        // fd 到 处理类* 的 映射
        std::array<Item *, 40000> fdMappings;

        // epoll_wait 事件存储
        std::array<epoll_event, 4096> events;

        // 存储的 epoll fd
        int efd = -1;


        // 其他线程封送到 epoll 线程的函数容器
        std::deque<std::function<void()>> actions;

        // 用于锁 actions
        std::mutex actionsMutex;

        // 0 读 1 写 用于 dispatch 时通知 epoll_wait
        std::array<int, 2> actionsPipes;

        // 当前正要执行的函数
        std::function<void()> action;



        // 时间轮. 只存指针引用, 不管理内存
        std::vector<ItemTimeout *> wheel;

        // 指向时间轮的游标
        int cursor = 0;


        /********************************************************/
        // 下面这几个用户可以读

        // 对于一些返回值非 int 的函数, 具体错误码将存放于此
        int lastErrorNumber = 0;

        // 公共只读: 每帧开始时更新一下
        int64_t nowMS = 0;

        // Run 时填充, 以便于局部获取并转换时间单位
        double frameRate = 1;


        /********************************************************/
        // 下面这几个可读写

        // 映射通过 stdin 进来的指令的处理函数. 去空格 去 tab 后第一个单词作为 key. 剩余部分作为 args
        std::unordered_map<std::string, std::function<void(std::vector<std::string> const &args)>> cmds;


        // 执行标志位。如果要退出，修改它
        bool running = true;


        // 参数：时间轮长度( 要求为 2^n )
        Context(size_t const &wheelLen = 1 << 12);

        virtual ~Context();

        Context(Context const &) = delete;

        Context &operator=(Context const &) = delete;


        // 创建非阻塞 socket fd 并返回. < 0: error
        int MakeSocketFD(int const &port, int const &sockType = SOCK_STREAM); // SOCK_DGRAM

        // 添加 fd 到 epoll 监视. return !0: error
        int Ctl(int const &fd, uint32_t const &flags, int const &op = EPOLL_CTL_ADD) const;

        // 关闭并从 epoll 移除监视
        int CloseDel(int const &fd) const;

        // 进入一次 epoll wait. 可传入超时时间.
        int Wait(int const &timeoutMS);


        // 每帧调用一次 以驱动 timer
        inline void UpdateTimeoutWheel();

        // 添加对象到容器
        template<typename T>
        T *AddItem(std::unique_ptr<T> &&item, int const &fd = -1);


        // 执行所有 Dispatch 的函数
        int HandleActions();



        /********************************************************/
        // 下面是外部主要使用的函数

        // 将秒转为帧数
        inline int SecondsToFrames(double const &sec) const { return (int) (frameRate * sec); }

        // 将毫秒转为帧数
        [[maybe_unused]] inline int MsToFrames(int const &ms) const { return (int) (frameRate * ms / 1000); }


        // 帧逻辑可以覆盖这个函数. 返回非 0 将令 Run 退出.
        inline virtual int FrameUpdate() { return 0; }


        // 开始运行并尽量维持在指定帧率. 临时拖慢将补帧
        int Run(double const &frameRate = 10);

        // 封送函数到 epoll 线程执行( 线程安全 ). 可能会阻塞.
        int Dispatch(std::function<void()>&& action);

        // 创建 TCP 监听器
        template<typename T = TcpListener, typename ...Args>
        Ref<T> CreateTcpListener(int const &port, Args &&... args);

        // 创建 拨号器
        template<typename T = TcpDialer, typename ...Args>
        Ref<T> CreateTcpDialer(Args &&... args);

        // 创建 定时器
        template<typename T = Timer, typename ...Args>
        [[maybe_unused]] Ref<T>
        CreateTimer(int const &interval, std::function<void(Timer_r const &timer)> &&cb, Args &&...args);


        // 启用命令行输入控制. 支持方向键, tab 补齐, 上下历史
        CommandHandler *EnableCommandLine();
    };



    /***********************************************************************************************************/
    // Util funcs
    /***********************************************************************************************************/

    // ip, port 转为 addr
    int FillAddress(std::string const &ip, int const &port, sockaddr_in6 &addr);


    std::string AddressToString(sockaddr *const &in);

    [[maybe_unused]] std::string AddressToString(sockaddr_in const &in);

    std::string AddressToString(sockaddr_in6 const &in);








    /***********************************************************************************************************/
    // Item
    /***********************************************************************************************************/

    inline void Item::Dispose() {
        if (indexAtContainer != -1) {
            // 因为 gcc 傻逼，此处由于要自杀，故先复制参数到栈，避免出异常
            auto e = this->ep;
            auto i = this->indexAtContainer;
            e->items.RemoveAt(i);    // 触发析构
        }
    }

    inline Item::~Item() {
        if (fd != -1) {
            assert(ep);
            ep->fdMappings[fd] = nullptr;
            ep->CloseDel(fd);
            fd = -1;
        }
    }

    /***********************************************************************************************************/
    // Ref
    /***********************************************************************************************************/

    template<typename T>
    inline Ref<T>::Ref(T *const &ptr) {
        static_assert(std::is_base_of_v<Item, T>);
        Reset(ptr);
    }

    template<typename T>
    inline Ref<T>::Ref(std::unique_ptr<T> const &ptr) : Ref(ptr.get()) {}

    template<typename T>
    inline Ref<T>::Ref(Ref &&o) noexcept
            : items(o.items), index(o.index), version(o.version) {
        o.items = nullptr;
        o.index = -1;
        o.version = 0;
    }

    template<typename T>
    Ref<T> &Ref<T>::operator=(Ref &&o) noexcept {
        std::swap(items, o.items);
        std::swap(index, o.index);
        std::swap(version, o.version);
        return *this;
    }


    template<typename T>
    template<typename U>
    Ref<T> &Ref<T>::operator=(std::enable_if_t<std::is_base_of_v<T, U>, Ref<U>> const &o) {
        operator=(*(Ref<T> *) &o);
        return *this;
    }

    template<typename T>
    template<typename U>
    Ref<T> &Ref<T>::operator=(std::enable_if_t<std::is_base_of_v<T, U>, Ref<U>> &&o) {
        operator=(std::move(*(Ref<T> *) &o));
        return *this;
    }


    template<typename T>
    template<typename U>
    inline Ref<U> Ref<T>::As() const {
        if (!dynamic_cast<U *>(Lock())) return Ref<U>();
        return *(Ref<U> *) this;
    }


    template<typename T>
    inline Ref<T>::operator bool() const {
        return version && items->VersionAt(index) == version;
    }

    template<typename T>
    inline T *Ref<T>::operator->() const {
        if (!operator bool())
            throw std::logic_error(
                    std::string("error: nullptr -> at line number:") + std::to_string(__LINE__));        // 空指针
        return (T *) items->ValueAt(index).get();
    }

    template<typename T>
    inline T *Ref<T>::Lock() const {
        return operator bool() ? (T *) items->ValueAt(index).get() : nullptr;
    }

    template<typename T>
    template<typename U>
    inline void Ref<T>::Reset(U *const &ptr) {
        static_assert(std::is_base_of_v<T, U>);
        if (!ptr) {
            items = nullptr;
            index = -1;
            version = 0;
        } else {
            assert(ptr->ep);
            assert(ptr->indexAtContainer != -1);
            items = &ptr->ep->items;
            index = ptr->indexAtContainer;
            version = items->VersionAt(index);
        }
    }


    /***********************************************************************************************************/
    // ItemTimeout
    /***********************************************************************************************************/

    // 返回非 0 表示找不到 管理器 或 参数错误
    inline int ItemTimeout::SetTimeout(int const &interval) {
        assert((int) ep->wheel.size() > interval);

        // 试着从 wheel 链表中移除
        if (timeoutIndex != -1) {
            if (timeoutNext != nullptr) {
                timeoutNext->timeoutPrev = timeoutPrev;
            }
            if (timeoutPrev != nullptr) {
                timeoutPrev->timeoutNext = timeoutNext;
            } else {
                ep->wheel[timeoutIndex] = timeoutNext;
            }
        }

        // 检查是否传入间隔时间
        if (interval) {
            // 如果设置了新的超时时间, 则放入相应的链表
            // 安全检查
            if (interval < 0 || interval > (int) ep->wheel.size()) return -2;

            // 环形定位到 wheel 元素目标链表下标
            timeoutIndex = (interval + ep->cursor) & ((int) ep->wheel.size() - 1);

            // 成为链表头
            timeoutPrev = nullptr;
            timeoutNext = ep->wheel[timeoutIndex];
            ep->wheel[timeoutIndex] = this;

            // 和之前的链表头连起来( 如果有的话 )
            if (timeoutNext) {
                timeoutNext->timeoutPrev = this;
            }
        } else {
            // 重置到初始状态
            timeoutIndex = -1;
            timeoutPrev = nullptr;
            timeoutNext = nullptr;
        }

        return 0;
    }

    inline int ItemTimeout::SetTimeoutSeconds(double const &seconds) {
        return SetTimeout(ep->SecondsToFrames(seconds));
    }

    inline ItemTimeout::~ItemTimeout() {
        if (timeoutIndex != -1) {
            SetTimeout(0);
        }
    }

    /***********************************************************************************************************/
    // Timer
    /***********************************************************************************************************/

    inline void Timer::OnTimeout() {
        if (onFire) {
            Ref<Timer> alive(this);    // 防止在 onFire 中 Dispose timer
            onFire(this);
            if (!alive) return;
        }
        // 非 repeat 模式( 未再次 SetTimeout )直接自杀
        if (timeoutIndex == -1) {
            Dispose();
        }
    }


    /***********************************************************************************************************/
    // TcpPeer
    /***********************************************************************************************************/

    template<typename T>
    Ref<T> TcpPeer::CreateByFD() {
        // 解除映射关系( 避免 AddItem 报错 )
        ep->fdMappings[fd] = nullptr;

        // 新建 T 实例 放入容器
        auto p = ep->AddItem(xx::TryMakeU<T>(), fd);

        // 填充 ip
        memcpy(&p->addr, &addr, sizeof(addr));

        // 设为 -1 以绕开析构函数中的 close
        this->fd = -1;

        // 返回新对象 的 弱引用
        return p;
    }

    inline void TcpPeer::OnDisconnect(int const &reason) {}

    inline void TcpPeer::OnReceive() {
        Send(recv.buf, recv.len);
        recv.Clear();
    }

    inline void TcpPeer::OnTimeout() {
        Ref<TcpPeer> alive(this);
        OnDisconnect(__LINE__);
        if (alive) {
            Dispose();
        }
    }

    inline void TcpPeer::OnEpollEvent(uint32_t const &e) {
        // error
        if (e & EPOLLERR || e & EPOLLHUP) {
            Ref<TcpPeer> alive(this);
            OnDisconnect(__LINE__);
            if (alive) {
                Dispose();
            }
            return;
        }
        // read
        if (e & EPOLLIN) {
            // 如果接收缓存没容量就扩容( 通常发生在首次使用时 )
            if (!recv.cap) {
                recv.Reserve(readBufLen);
            }

            // 如果数据长度 == buf限长 就自杀( 未处理数据累计太多? )
            if (recv.len == recv.cap) {
                Ref<TcpPeer> alive(this);
                OnDisconnect(__LINE__);
                if (alive) {
                    Dispose();
                }
                return;
            }

            // 通过 fd 从系统网络缓冲区读取数据. 追加填充到 recv 后面区域. 返回填充长度. <= 0 则认为失败 自杀
            auto &&len = read(fd, recv.buf + recv.len, recv.cap - recv.len);
            if (len <= 0) {
                Ref<TcpPeer> alive(this);
                OnDisconnect(__LINE__);
                if (alive) {
                    Dispose();
                }
                return;
            }
            recv.len += len;

            // 调用用户数据处理函数
            {
                Ref<TcpPeer> alive(this);
                OnReceive();
                if (!alive) return;
            }
        }
        // write
        if (e & EPOLLOUT) {
            // 设置为可写状态
            writing = false;
            if (int r = Write()) {
                ep->lastErrorNumber = r;
                Ref<TcpPeer> alive(this);
                OnDisconnect(__LINE__);
                if (alive) {
                    Dispose();
                }
                return;
            }
        }
    }

    inline int TcpPeer::Send(char const *const &buf, size_t const &len) {
        sendQueue.Push(Data(buf, len));
        return !writing ? Write() : 0;
    }

    inline int TcpPeer::Send(Data &&data) {
        sendQueue.Push(std::move(data));
        return !writing ? Write() : 0;
    }

    inline int TcpPeer::Write() {
        // 如果没有待发送数据，停止监控 EPOLLOUT 并退出
        if (!sendQueue.bytes) return ep->Ctl(fd, EPOLLIN, EPOLL_CTL_MOD);

        // 前置准备
        std::array<iovec, UIO_MAXIOV> vs;                     // buf + len 数组指针
        int vsLen = 0;                                        // 数组长度
        auto bufLen = sendLenPerFrame;                        // 计划发送字节数

        // 填充 vs, vsLen, bufLen 并返回预期 offset. 每次只发送 bufLen 长度
        auto &&offset = sendQueue.Fill(vs, vsLen, bufLen);

        // 返回值为 实际发出的字节数
        auto &&sentLen = writev(fd, vs.data(), vsLen);

        // 已断开
        if (sentLen == 0) return -2;

            // 繁忙 或 出错
        else if (sentLen == -1) {
            if (errno == EAGAIN) goto LabEnd;
            else return -3;
        }

            // 完整发送
        else if ((size_t) sentLen == bufLen) {
            // 快速弹出已发送数据
            sendQueue.Pop(vsLen, offset, bufLen);

            // 这次就写这么多了. 直接返回. 下次继续 Write
            return 0;
        }

            // 发送了一部分
        else {
            // 弹出已发送数据
            sendQueue.Pop(sentLen);
        }

        LabEnd:
        // 标记为不可写
        writing = true;

        // 开启对可写状态的监控, 直到队列变空再关闭监控
        return ep->Ctl(fd, EPOLLIN | EPOLLOUT, EPOLL_CTL_MOD);
    }



    /***********************************************************************************************************/
    // TcpListener
    /***********************************************************************************************************/

    inline TcpPeer_u TcpListener::OnCreatePeer() {
        return xx::TryMakeU<TcpPeer>();
    }

    inline void TcpListener::OnEpollEvent(uint32_t const &e) {
        // error
        if (e & EPOLLERR || e & EPOLLHUP) {
            Dispose();
            return;
        }
        // accept 到 没有 或 出错 为止
        while (Accept(fd) > 0) {};
    }

    inline int TcpListener::Accept(int const &listenFD) {
        // 开始创建 fd
        sockaddr_in6 addr;
        socklen_t len = sizeof(addr);

        // 接收并得到目标 fd
        int fd = accept(listenFD, (sockaddr *) &addr, &len);
        if (-1 == fd) {
            ep->lastErrorNumber = errno;
            if (ep->lastErrorNumber == EAGAIN || ep->lastErrorNumber == EWOULDBLOCK) return 0;
            else return -1;
        }

        // 确保退出时自动关闭 fd
        ScopeGuard sg([&] { close(fd); });

        // 如果 fd 超出最大存储限制就退出。返回 fd 是为了外部能继续执行 accept
        if (fd >= (int) ep->fdMappings.size()) return fd;
        assert(!ep->fdMappings[fd]);

        // 设置非阻塞状态
        if (-1 == fcntl(fd, F_SETFL, fcntl(fd, F_GETFL, 0) | O_NONBLOCK)) return -2;

        // 设置一些 tcp 参数( 可选 )
        int on = 1;
        if (-1 == setsockopt(fd, IPPROTO_TCP, TCP_NODELAY, (const char *) &on, sizeof(on))) return -3;

        // 纳入 ep 管理
        if (-1 == ep->Ctl(fd, EPOLLIN)) return -4;

        // 确保退出时关闭 fd 并从 epoll 监视移除
        sg.Set([&] { ep->CloseDel(fd); });


        // 创建类容器. 允许创建失败( 比如内存不足，或者刻意控制数量 ). 失败不触发 OnAccept
        auto u = OnCreatePeer();
        if (!u) return fd;

        // 放入容器并拿到指针用, 继续填充
        auto p = ep->AddItem(std::move(u), fd);

        // 填充 ip
        memcpy(&p->addr, &addr, len);

        // 撤销自动关闭
        sg.Cancel();

        // 初始化
        Ref<TcpPeer> alive(p);
        p->Init();
        if (!alive) return fd;

        // 触发事件
        OnAccept(p);
        return fd;
    }



    /***********************************************************************************************************/
    // TcpConn
    /***********************************************************************************************************/

    inline void TcpConn::OnEpollEvent(uint32_t const &e) {
        // 如果 dialer 无法定位到 或 error 事件则自杀
        auto d = this->dialer.Lock();
        if (!d || e & EPOLLERR || e & EPOLLHUP) {
            Dispose();
            return;
        }
        // 读取错误 或者读到错误 都认为是连接失败. 返回非 0 触发 Dispose
        int err;
        socklen_t result_len = sizeof(err);
        if (getsockopt(fd, SOL_SOCKET, SO_ERROR, &err, &result_len) == -1 || err) {
            Dispose();
            return;
        }

        // 连接成功. 自杀前备份变量到栈
        auto fd = this->fd;
        auto ep = this->ep;
        // 设为 -1 以绕开析构函数中的 close
        this->fd = -1;
        // 清除映射关系
        ep->fdMappings[fd] = nullptr;
        Dispose();    // 自杀

        // 这之后只能用 "栈"变量
        d->Stop();
        auto peer = d->OnCreatePeer();
        if (peer) {
            auto p = ep->AddItem(std::move(peer), fd);    // peer is moved
            // fill address
            result_len = sizeof(p->addr);
            getpeername(fd, (sockaddr *) &p->addr, &result_len);
            d->OnConnect(p);
        } else {
            ep->CloseDel(fd);
            d->OnConnect(d->emptyPeer);
        }
    }



    /***********************************************************************************************************/
    // TcpDialer
    /***********************************************************************************************************/

    inline int TcpDialer::AddAddress(std::string const &ip, int const &port) {
        auto &&a = addrs.emplace_back();
        if (int r = FillAddress(ip, port, a)) {
            addrs.pop_back();
            return r;
        }
        return 0;
    }

    inline int TcpDialer::Dial(int const &timeoutFrames) {
        Stop();
        SetTimeout(timeoutFrames);
        for (auto &&a : addrs) {
            if (int r = NewTcpConn(a)) {
                Stop();
                return r;
            }
        }
        return 0;
    }

    inline int TcpDialer::DialSeconds(double const &timeoutSeconds) {
        return Dial(ep->SecondsToFrames(timeoutSeconds));
    }

    inline bool TcpDialer::Busy() {
        // 用超时检测判断是否正在拨号
        return timeoutIndex != -1;
    }

    inline void TcpDialer::Stop() {
        // 清理原先的残留
        for (auto &&conn : conns) {
            if (auto &&c = conn.Lock()) {
                c->Dispose();
            }
        }
        conns.clear();

        // 清除超时检测
        SetTimeout(0);
    }

    inline void TcpDialer::OnTimeout() {
        Stop();
        OnConnect(emptyPeer);
    }

    inline TcpDialer::~TcpDialer() {
        Stop();
    }

    inline TcpPeer_u TcpDialer::OnCreatePeer() {
        return xx::TryMakeU<TcpPeer>();
    }

    inline int TcpDialer::NewTcpConn(sockaddr_in6 const &addr) {
        // 创建 tcp 非阻塞 socket fd
        auto &&fd = socket(AF_INET, SOCK_STREAM | SOCK_NONBLOCK, 0);
        if (fd == -1) return -1;

        // 确保 return 时自动 close
        xx::ScopeGuard sg([&] { close(fd); });

        // 检测 fd 存储上限
        if (fd >= (int) ep->fdMappings.size()) return -2;
        assert(!ep->fdMappings[fd]);

        // 设置一些 tcp 参数( 可选 )
        int on = 1;
        if (-1 == setsockopt(fd, IPPROTO_TCP, TCP_NODELAY, (const char *) &on, sizeof(on))) return -3;

        // 开始连接
        if (connect(fd, (sockaddr *) &addr, sizeof(addr)) == -1) {
            if (errno != EINPROGRESS) return -4;
        }
        // else : 立刻连接上了

        // 纳入 epoll 管理
        if (int r = ep->Ctl(fd, EPOLLIN | EPOLLOUT)) return r;

        // 确保 return 时自动 close 并脱离 epoll 管理
        sg.Set([&] { ep->CloseDel(fd); });

        // 试创建目标类实例
        auto o_ = xx::TryMakeU<TcpConn>();
        if (!o_) return -5;

        // 继续初始化并放入容器
        auto o = ep->AddItem(std::move(o_), fd);
        o->dialer = this;
        conns.emplace_back(o);

        sg.Cancel();
        o->Init();
        return 0;
    }



    /***********************************************************************************************************/
    // CommandHandler
    /***********************************************************************************************************/

    inline CommandHandler::CommandHandler() {
        // 初始化 readline 第三方组件. 通过静态函数 访问静态 self 类 从而调用类成员函数
        self = this;
        rl_attempted_completion_function = (rl_completion_func_t *) &CompleteCallback;
        rl_callback_handler_install("# ", (rl_vcpfunc_t *) &ReadLineCallback);
    }

    inline void CommandHandler::Exec(char const *const &row, size_t const &len) {
        // 读取 row 内容, call ep->cmds[ args[0] ]( args )
        args.clear();
        std::string s;
        bool jumpSpace = true;
        for (size_t i = 0; i < len; ++i) {
            auto c = row[i];
            if (jumpSpace) {
                if (c != '	' && c != ' '/* && c != 10*/) {
                    s += c;
                    jumpSpace = false;
                }
                continue;
            } else if (c == '	' || c == ' '/* || c == 10*/) {
                args.emplace_back(std::move(s));
                jumpSpace = true;
                continue;
            } else {
                s += c;
            }
        }
        if (!s.empty()) {
            args.emplace_back(std::move(s));
        }

        if (!args.empty()) {
            auto &&iter = ep->cmds.find(args[0]);
            if (iter != ep->cmds.end()) {
                if (iter->second) {
                    iter->second(args);
                }
            } else {
                std::cout << "unknown command: " << args[0] << std::endl;
            }
        }
    }

    inline void CommandHandler::ReadLineCallback(char *line) {
        if (!line) return;
        auto len = strlen(line);
        if (len) {
            add_history(line);
        }
        self->Exec(line, len);
        free(line);
    }

    inline char *CommandHandler::CompleteGenerate(const char *t, int state) {
        static std::vector<std::string> matches;
        static size_t match_index = 0;

        // 如果是首次回调 则开始填充对照表
        if (state == 0) {
            matches.clear();
            match_index = 0;

            std::string s(t);
            for (auto &&kv : self->ep->cmds) {
                auto &&word = kv.first;
                if (word.size() >= s.size() &&
                    word.compare(0, s.size(), s) == 0) {
                    matches.push_back(word);
                }
            }
        }

        // 没找到
        if (match_index >= matches.size()) return nullptr;

        // 找到: 克隆一份返回
        return strdup(matches[match_index++].c_str());
    }

    inline char **
    CommandHandler::CompleteCallback(const char *text, [[maybe_unused]] int start, [[maybe_unused]] int end) {
        // 不做文件名适配
        rl_attempted_completion_over = 1;
        return rl_completion_matches(text, (rl_compentry_func_t *) &CompleteGenerate);
    }

    inline void CommandHandler::OnEpollEvent(uint32_t const &e) {
        // error
        if (e & EPOLLERR || e & EPOLLHUP) {
            Dispose();
            return;
        }

        rl_callback_read_char();
    }

    inline CommandHandler::~CommandHandler() {
        rl_callback_handler_remove();
        rl_clear_history();

        epoll_ctl(ep->efd, EPOLL_CTL_DEL, fd, nullptr);
        ep->fdMappings[fd] = nullptr;
        fd = -1;
    }


    /***********************************************************************************************************/
    // PipeReader
    /***********************************************************************************************************/

    void PipeReader::OnEpollEvent(uint32_t const &e) {
        ep->HandleActions();
    }


    /***********************************************************************************************************/
    // Context
    /***********************************************************************************************************/

    inline Context::Context(size_t const &wheelLen) {
        // 创建 epoll fd
        efd = epoll_create1(0);
        if (-1 == efd) throw std::logic_error("create epoll context failed.");

        // 创建 pipe fd
        if (pipe(actionsPipes.data())) {
            close(efd);
            throw std::logic_error("create pipe context failed.");;
        }

        // 设置 pipe 为非阻塞( 当前逻辑因为是只要进入一次 HandleActions 就执行光队列里的函数, 故写失败可忽略 )
        fcntl(actionsPipes[1], F_SETFL, O_NONBLOCK);
        fcntl(actionsPipes[0], F_SETFL, O_NONBLOCK);

        // 将 pipe fd 纳入 epoll 管理
        Ctl(actionsPipes[0], EPOLLIN);

        // 为 pipe fd[0] 创建处理 item
        AddItem(xx::MakeU<PipeReader>(), actionsPipes[0]);


        // 初始化时间伦
        wheel.resize(wheelLen);

        // 初始化处理类映射表
        fdMappings.fill(nullptr);
    }

    inline Context::~Context() {
        // 所有 items 析构
        items.Clear();

        // 关闭 epoll 本身 fd
        if (efd != -1) {
            close(efd);
            efd = -1;
        }

        // 关闭 pipe fd( [0] 在 item 自杀时已经关闭 )
        if (actionsPipes[1]) {
            close(actionsPipes[1]);
            actionsPipes.fill(0);
        }
    }

    inline int Context::MakeSocketFD(int const &port, int const &sockType) {
        char portStr[20];
        snprintf(portStr, sizeof(portStr), "%d", port);

        addrinfo hints;
        memset(&hints, 0, sizeof(addrinfo));
        hints.ai_family = AF_UNSPEC;                                        // ipv4 / 6
        hints.ai_socktype = sockType;                                        // SOCK_STREAM / SOCK_DGRAM
        hints.ai_flags = AI_PASSIVE;                                        // all interfaces

        addrinfo *ai_, *ai;
        if (getaddrinfo(nullptr, portStr, &hints, &ai_)) return -1;

        int fd;
        for (ai = ai_; ai != nullptr; ai = ai->ai_next) {
            fd = socket(ai->ai_family, ai->ai_socktype | SOCK_NONBLOCK, ai->ai_protocol);
            if (fd == -1) continue;

            int enable = 1;
            if (setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, &enable, sizeof(int)) < 0) {
                close(fd);
                continue;
            }
            if (!bind(fd, ai->ai_addr, ai->ai_addrlen)) break;                // success

            close(fd);
        }
        freeaddrinfo(ai_);

        if (!ai) return -2;

        // 检测 fd 存储上限
        if (fd >= (int) fdMappings.size()) {
            close(fd);
            return -3;
        }
        assert(!fdMappings[fd]);

        return fd;
    }

    inline int Context::Ctl(int const &fd, uint32_t const &flags, int const &op) const {
        epoll_event event;
        bzero(&event, sizeof(event));
        event.data.fd = fd;
        event.events = flags;
        return epoll_ctl(efd, op, fd, &event);
    };

    inline int Context::CloseDel(int const &fd) const {
        assert(fd != -1);
        epoll_ctl(efd, EPOLL_CTL_DEL, fd, nullptr);
        return close(fd);
    }

    inline int Context::Wait(int const &timeoutMS) {
        int n = epoll_wait(efd, events.data(), (int) events.size(), timeoutMS);
        if (n == -1) return errno;
        for (int i = 0; i < n; ++i) {
            auto fd = events[i].data.fd;
            auto h = fdMappings[fd];
            if (!h) {
                std::cout << "epoll wait !h. fd = " << fd << std::endl;
                assert(h);
            }
            assert(!h->fd || h->fd == fd);
            auto e = events[i].events;
            h->OnEpollEvent(e);
        }
        return 0;
    }

    inline void Context::UpdateTimeoutWheel() {
        cursor = (cursor + 1) & ((int) wheel.size() - 1);
        auto p = wheel[cursor];
        wheel[cursor] = nullptr;
        while (p) {
            auto np = p->timeoutNext;

            p->timeoutIndex = -1;
            p->timeoutPrev = nullptr;
            p->timeoutNext = nullptr;

            p->OnTimeout();
            p = np;
        };
    }

    inline int Context::HandleActions() {
        char buf[512];
        if (read(actionsPipes[0], buf, sizeof(buf)) <= 0) return -1;
        while (true) {
            {
                std::scoped_lock<std::mutex> g(actionsMutex);
                if (actions.empty()) break;
                action = std::move(actions.front());
                actions.pop_front();
            }
            action();
            action = nullptr;
        }
        return 0;
    }



    inline int Context::Run(double const &frameRate_) {
        assert(frameRate_ > 0);
        this->frameRate = frameRate_;

        // 稳定帧回调用的时间池
        double ticksPool = 0;

        // 本次要 Wait 的超时时长
        int waitMS = 0;

        // 计算帧时间间隔
        auto ticksPerFrame = 10000000.0 / frameRate_;

        // 取当前时间
        auto lastTicks = xx::NowEpoch10m();

        // 更新一下逻辑可能用到的时间戳
        nowMS = xx::NowSteadyEpochMS();

        // 开始循环
        while (running) {

            // 计算上个循环到现在经历的时长, 并累加到 pool
            auto currTicks = xx::NowEpoch10m();
            auto elapsedTicks = (double) (currTicks - lastTicks);
            ticksPool += elapsedTicks;
            lastTicks = currTicks;

            // 如果累计时长跨度大于一帧的时长 则 Update
            if (ticksPool > ticksPerFrame) {

                // 消耗累计时长
                ticksPool -= ticksPerFrame;

                // 本次 Wait 不等待.
                waitMS = 0;

                // 更新一下逻辑可能用到的时间戳
                nowMS = xx::NowSteadyEpochMS();

                // 驱动 timerswfffffff
                UpdateTimeoutWheel();

                // 帧逻辑调用一次
                if (int r = FrameUpdate()) return r;
            } else {
                // 计算等待时长
                waitMS = (int) ((ticksPerFrame - elapsedTicks) / 10000.0);
            }

            // 调用一次 epoll wait.
            if (int r = Wait(waitMS)) return r;
        }

        return 0;
    }


    inline int Context::Dispatch(std::function<void()>&& action) {
        {
            std::scoped_lock<std::mutex> g(actionsMutex);
            actions.push_back(std::move(action));
        }
        return write(actionsPipes[1], ".", 1) == 1 ? -1 : 0;
    }


    template<typename L, typename ...Args>
    inline Ref<L> Context::CreateTcpListener(int const &port, Args &&... args) {
        static_assert(std::is_base_of_v<TcpListener, L>);

        // 创建监听用 socket fd
        auto &&fd = MakeSocketFD(port);
        if (fd < 0) {
            lastErrorNumber = -1;
            return Ref<L>();
        }
        // 确保 return 时自动 close
        xx::ScopeGuard sg([&] { close(fd); });

        // 开始监听
        if (-1 == listen(fd, SOMAXCONN)) {
            lastErrorNumber = -3;
            return Ref<L>();
        }

        // fd 纳入 epoll 管理
        if (-1 == Ctl(fd, EPOLLIN)) {
            lastErrorNumber = -4;
            return Ref<L>();
        }

        // 确保 return 时自动 close 并脱离 epoll 管理
        sg.Set([&] { CloseDel(fd); });

        // 试创建目标类实例
        auto o_ = xx::TryMakeU<L>(std::forward<Args>(args)...);
        if (!o_) {
            lastErrorNumber = -5;
            return Ref<L>();
        }

        // 撤销自动关闭行为并返回结果
        sg.Cancel();

        // 放入容器, 初始化并返回
        auto o = AddItem(std::move(o_), fd);
        o->Init();
        return o;
    }


    template<typename TD, typename ...Args>
    inline Ref<TD> Context::CreateTcpDialer(Args &&... args) {
        static_assert(std::is_base_of_v<TcpDialer, TD>);

        // 试创建目标类实例
        auto o_ = xx::TryMakeU<TD>(std::forward<Args>(args)...);
        if (!o_) {
            lastErrorNumber = -1;
            return Ref<TD>();
        }

        // 放入容器, 初始化并返回
        auto o = AddItem(std::move(o_));
        o->Init();
        return o;
    }


    template<typename T, typename ...Args>
    [[maybe_unused]] [[maybe_unused]] inline Ref<T>
    Context::CreateTimer(int const &interval, std::function<void(Timer_r const &timer)> &&cb, Args &&...args) {
        static_assert(std::is_base_of_v<Timer, T>);

        // 试创建目标类实例
        auto o_ = xx::TryMakeU<T>(std::forward<Args>(args)...);
        if (!o_) {
            lastErrorNumber = -1;
            return Ref<T>();
        }

        // 放入容器
        auto o = AddItem(std::move(o_));

        // 设置超时时长
        if (o->SetTimeout(interval)) {
            o->Dispose();
            return Ref<T>();
        }

        // 设置回调
        o->onFire = std::move(cb);

        // 初始化
        o->Init();
        return o;
    }


    inline int FillAddress(std::string const &ip, int const &port, sockaddr_in6 &addr) {
        memset(&addr, 0, sizeof(addr));

        if (ip.find(':') == std::string::npos) {        // ipv4
            auto a = (sockaddr_in *) &addr;
            a->sin_family = AF_INET;
            a->sin_port = htons((uint16_t) port);
            if (!inet_pton(AF_INET, ip.c_str(), &a->sin_addr)) return -1;
        } else {                                            // ipv6
            auto a = &addr;
            a->sin6_family = AF_INET6;
            a->sin6_port = htons((uint16_t) port);
            if (!inet_pton(AF_INET6, ip.c_str(), &a->sin6_addr)) return -1;
        }

        return 0;
    }


    template<typename T>
    inline T *Context::AddItem(std::unique_ptr<T> &&item, int const &fd) {
        static_assert(std::is_base_of_v<Item, T>);
        assert(item);
        auto p = item.get();
        p->indexAtContainer = items.Add(std::move(item));
        p->ep = this;
        if (fd != -1) {
            assert(!fdMappings[fd]);
            p->fd = fd;
            fdMappings[fd] = p;
        }
        return p;
    }


    inline CommandHandler *Context::EnableCommandLine() {
        // 已存在：直接短路返回
        if (fdMappings[STDIN_FILENO]) return (CommandHandler *) fdMappings[STDIN_FILENO];

        // 试将 fd 纳入 epoll 管理
        if (-1 == Ctl(STDIN_FILENO, EPOLLIN)) return nullptr;

        // 创建 stdin fd 的处理类并放入容器
        return AddItem(xx::MakeU<CommandHandler>(), STDIN_FILENO);
    }


    inline std::string AddressToString(sockaddr *const &in) {
        if (in) {
            char hbuf[NI_MAXHOST], sbuf[NI_MAXSERV];
            if (!getnameinfo(in, in->sa_family == AF_INET6 ? INET6_ADDRSTRLEN : INET_ADDRSTRLEN, hbuf, sizeof hbuf,
                             sbuf,
                             sizeof sbuf, NI_NUMERICHOST | NI_NUMERICSERV)) {
                return std::string(hbuf) + ":" + sbuf;
            }
        }
        return "";
    }

    [[maybe_unused]] [[maybe_unused]] inline std::string AddressToString(sockaddr_in const &in) {
        return AddressToString((sockaddr *) &in);
    }

    inline std::string AddressToString(sockaddr_in6 const &in) {
        return AddressToString((sockaddr *) &in);
    }
}
