/*
    注意：得到 Peer 后可以例行 Hold(), 释放时 DelayUnhold(). 直接干掉容器导致析构是不安全的.
    基类构造&析构中 无法调用 派生类的 override( 没映射 ), 无法 shared_from_this( 构造万一 throw 则别处通过 ptr 能执行到析构 )
*/

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

#include "xx_typehelpers.h"
#include "xx_chrono.h"
#include "xx_data_queue.h"
#include "xx_scopeguard.h"

#define LIKELY(x)       __builtin_expect(!!(x), 1)
#define UNLIKELY(x)     __builtin_expect(!!(x), 0)

namespace xx::Epoll {
    /***********************************************************************************************************/
    // Item
    // 所有受 Context 管理的子类的基类. 主要特征为其 fd 受 context 中的 epoll 管理
    struct Context;
    struct Item : std::enable_shared_from_this<Item> {
        // 指向总容器
        std::shared_ptr<Context> ec;
        // linux 系统文件描述符
        int fd;

        // 初始化依赖上下文
        explicit Item(std::shared_ptr<Context> const& ec, int const& fd = -1);
        // 注意：析构中调用虚函数，不会 call 到派生类的 override 版本
        virtual ~Item() { Close(0); }
        // 会导致 关闭 fd 解除映射, fd = -1. reason 通常传 __LINE__. 如果传 0 则通常由 非基类析构 发起
        virtual bool Close(int const& reason);
        // 将当前实例的智能指针放入 ec->holdItems( 不能在构造函数或析构中执行 )
        void Hold();
        // 将当前实例的指针放入 ec->deadItems( 不能在析构中执行 ) 稍后会从 ec->holdItems 移除以触发析构
        void DelayUnhold();
    protected:
        friend Context;
        // epoll 事件处理. 用不上不必实现
        inline virtual void EpollEvent(uint32_t const &e) {}
    };

    /***********************************************************************************************************/
    // Timer
    // 带超时的 Item
    struct Timer : Item {
    protected:
        friend Context;
        // 位于时间轮的下标
        int timeoutIndex = -1;
        // 指向上一个对象（在时间轮每一格形成一个双链表）
        Timer *timeoutPrev = nullptr;
        // 指向下一个对象（在时间轮每一格形成一个双链表）
        Timer *timeoutNext = nullptr;
    public:
        // 继承构造函数
        using Item::Item;
        // 设置超时( 单位：帧 )
        void SetTimeout(int const &interval);
        // 设置超时( 单位：秒 )
        void SetTimeoutSeconds(double const &seconds);
        // 从时间轮和链表中移除
        ~Timer() override;
        // 覆盖以实现超时后的逻辑. 如果要 repeat 效果，则可以函数内再次执行 SetTimeout
        virtual void Timeout() = 0;
    };

    /***********************************************************************************************************/
    // TcpPeer
    struct TcpPeer : Timer {
        // 对方的 addr( 连接建立，获取后缓存在此. 用户可读之 )
        sockaddr_in6 addr;
        // 收数据用堆积容器( Receive 里访问它来处理收到的数据 )
        Data recv;

        // 继承构造函数
        using Timer::Timer;
        // 数据接收事件: 用户可以从 recv 拿数据, 并移除掉已处理的部分
        virtual void Receive() = 0;
        // Data 对象移进队列并开始发送( 如果 fd == -1 则忽略 )
        int Send(Data &&data);
        // 会复制数据
        inline int Send(char const* const& buf, size_t const& len) { return Send({buf, len}); }
        // 判断 peer 是否还活着( 没断 )
        inline bool Alive() { return fd != -1; }
        // 和另一个 peer 互换 fd 和 mapping
        void SwapFD(std::shared_ptr<TcpPeer> const& o);
    protected:
        // 是否正在发送( 是：不管 sendQueue 空不空，都不能 write, 只能塞 sendQueue )
        bool writing = false;
        // 待发送队列
        xx::DataQueue sendQueue;
        // 发送
        int Write();
        // 超时触发 Close(__LINE__)
        void Timeout() override;
        // epoll 事件处理
        void EpollEvent(uint32_t const &e) override;
    };

    /***********************************************************************************************************/
    // TcpListener
    template<typename PeerType, class ENABLED = std::is_base_of<TcpPeer, PeerType>>
    struct TcpListener : Item {
        // 特化构造函数. 不需要传入 fd
        explicit TcpListener(std::shared_ptr<Context> const& ec) : Item(ec, -1) {};
        // 开始监听. 失败返回非 0
        int Listen(int const& port);
        // 提供为 peer 绑定事件的实现
        virtual void Accept(std::shared_ptr<PeerType> const &peer) = 0;
    protected:
        // 调用 accept
        void EpollEvent(uint32_t const &e) override;
        // return fd. <0: error. 0: empty (EAGAIN / EWOULDBLOCK), > 0: fd
        int HandleAccept(int const &listenFD);
    };

    /***********************************************************************************************************/
    // TcpConn

    template<typename PeerType, class ENABLED = std::is_base_of<TcpPeer, PeerType>>
    struct TcpDialer;

    // TcpDialer 产生的临时 fd 封装类. 连接成功后将 fd 移交给 TcpPeer
    template<typename PeerType, class ENABLED = std::is_base_of<TcpPeer, PeerType>>
    struct TcpConn : Item {
        // 指向拨号器, 方便调用其 Connect 函数
        std::shared_ptr<TcpDialer<PeerType>> dialer;
        // 继承构造函数
        using Item::Item;
        // 判断是否连接成功
        void EpollEvent(uint32_t const &e) override;
    };


    /***********************************************************************************************************/
    // TcpDialer
    // 拨号器
    template<typename PeerType, class ENABLED>
    struct TcpDialer : Timer {
        // 要连的地址数组. 带协议标记
        std::vector<sockaddr_in6> addrs;

        // 特化构造函数. 不需要传递 fd
        explicit TcpDialer(std::shared_ptr<Context> const& ec) : Timer(ec, -1) {}
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
        // 停止拨号 并清理 conns. 保留 addrs.
        void Stop();
        // 连接成功或超时后触发
        virtual void Connect(std::shared_ptr<PeerType> const &peer) = 0;
    protected:
        // 存个空值备用 以方便返回引用
        std::shared_ptr<PeerType> emptyPeer;
        // 内部连接对象. 拨号完毕后会被清空
        std::vector<std::shared_ptr<TcpConn<PeerType>>> conns;
        // 超时表明所有连接都没有连上. 触发 Connect( nullptr )
        void Timeout() override;
        // 根据地址创建一个 TcpConn
        int NewTcpConn(sockaddr_in6 const &addr);
    };

    /***********************************************************************************************************/
    // Pipe Reader & Writer
    // 通过 写 pipe 令 epoll_wait 退出( 用于跨线程 dispatch 通知 )
    struct PipeReader : Item {
        using Item::Item;
        void EpollEvent(uint32_t const &e) override;
    };
    struct PipeWriter : Item {
        using Item::Item;
        ~PipeWriter() override { close(fd); fd = -1; }
        inline int WriteSome() { return write(fd, ".", 1) == 1 ? -1 : 0; }
    };

    /***********************************************************************************************************/
    // CommandHandler
    // 处理键盘输入指令的专用类( 单例 ). 直接映射到 STDIN_FILENO ( fd == 0 ). 由首个创建的 epoll context 来注册和反注册
    struct CommandHandler : Item {
        inline static std::shared_ptr<CommandHandler> self;
        std::vector<std::string> args;
        CommandHandler(std::shared_ptr<Context> const& ec);
        static void ReadLineCallback(char *line);
        static char **CompleteCallback(const char *text, int start, int end);
        static char *CompleteGenerate(const char *text, int state);
        void EpollEvent(uint32_t const &e) override;
        ~CommandHandler() override;
        // 解析 row 内容并调用 cmd 绑定 handler
        void Exec(char const *const &row, size_t const &len);
    };

    /***********************************************************************************************************/
    // Context
    // 封装了 epoll 的 fd
    struct Context : std::enable_shared_from_this<Context> {
        // 执行标志位。如果要退出，修改它
        bool running = true;
        // 公共只读: 每帧开始时更新一下
        int64_t nowMS = 0;
        // Run 时填充, 以便于局部获取并转换时间单位
        double frameRate = 10;
        // 帧时间间隔
        double ticksPerFrame = 1.0 / frameRate;
        // 公用 buf( 需要的地方可临时用用 )
        std::array<char, 256 * 1024> buf;
        // 公用 data( 需要的地方可临时用用 )
        xx::Data data;
        // 映射通过 stdin 进来的指令的处理函数. 去空格 去 tab 后第一个单词作为 key. 剩余部分作为 args
        std::unordered_map<std::string, std::function<void(std::vector<std::string> const &args)>> cmds;

        // 参数：时间轮长度( 要求为 2^n )
        explicit Context(size_t const &wheelLen = (1u << 12u));
        Context(Context const &) = delete;
        Context &operator=(Context const &) = delete;
        virtual ~Context();

        // 帧逻辑可以覆盖这个函数. 返回非 0 将令 Run 退出
        inline virtual int FrameUpdate() { return 0; }
        // 初始化 Run 帧率
        virtual int SetFrameRate(double const &frameRate);
        // 开始运行并尽量维持在指定帧率. 临时拖慢将补帧
        virtual int Run();
        // 封送函数到 epoll 线程执行( 线程安全 ). 可能会阻塞.
        int Dispatch(std::function<void()>&& action);
        // 延迟执行一个函数( Wait 之后 )
        void DelayExecute(std::function<void()>&& func);

        // 启用命令行输入控制. 支持方向键, tab 补齐, 上下历史
        int EnableCommandLine();
        // 关闭命令行输入控制( 减持 Context 的引用计数 )
        void DisableCommandLine();

        /************************************************************************/
        // 下面的东西内部使用，别乱搞

        // fd 到 处理类* 的 映射
        std::array<Item*, 40000> fdMappings;
        // epoll_wait 事件存储
        std::array<epoll_event, 4096> events;
        // 存储的 epoll fd
        int efd = -1;
        // 延迟到 wait 之后执行的函数集合
        std::vector<std::function<void()>> delayFuncs;

        // 其他线程封送到 epoll 线程的函数容器
        std::deque<std::function<void()>> actions;
        // 用于锁 actions
        std::mutex actionsMutex;
        // 当前正要执行的函数
        std::function<void()> action;
        // pipe fd 容器( Run 内创建, Run 退出时析构 )
        std::shared_ptr<PipeReader> pipeReader;
        std::shared_ptr<PipeWriter> pipeWriter;

        // item 的智能指针的保持容器
        std::unordered_map<Item*, std::shared_ptr<Item>> holdItems;
        // 要删除一个 peer 就把它的 指针 压到这个队列. 会在 稍后 从 items 删除
        std::vector<Item*> deadItems;

        // 时间轮. 只存指针引用, 不管理内存
        std::vector<Timer*> wheel;
        // 指向时间轮的游标
        int cursor = 0;

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
        // 执行所有 Dispatch 的函数
        int HandleActions();
        // 将秒转为帧数
        inline int SecondsToFrames(double const &sec) const { return (int) (frameRate * sec); }
    };


    /***********************************************************************************************************/
    // Util funcs
    // ip, port 转为 addr
    int FillAddress(std::string const &ip, int const &port, sockaddr_in6 &addr);
    // addr 转为 string
    std::string AddressToString(sockaddr *const &in);
    std::string AddressToString(sockaddr_in6 const &in);


    /***********************************************************************************************************/
    // Item
    inline Item::Item(std::shared_ptr<Context> const& ec, int const& fd)
        : ec(ec)
        , fd(fd) {
        if (fd != -1) {
            assert(!ec->fdMappings[fd]);
            ec->fdMappings[fd] = this;
        }
    }

    inline bool Item::Close(int const& reason) {
        if (fd != -1) {
            assert(ec->fdMappings[fd] == this);
            // 解绑
            ec->fdMappings[fd] = nullptr;
            // 从 epoll 移除 并 关闭 fd
            ec->CloseDel(fd);
            // 防重入
            fd = -1;
            // 返回关闭成功
            return true;
        }
        // 返回关闭忽略
        return false;
    }

    inline void Item::Hold() {
        ec->holdItems[this] = shared_from_this();
    }

    inline void Item::DelayUnhold() {
        ec->deadItems.emplace_back(this);
    }


    /***********************************************************************************************************/
    // Timer
    // 返回非 0 表示找不到 管理器 或 参数错误
    inline void Timer::SetTimeout(int const &interval) {
        // 试着从 wheel 链表中移除
        if (timeoutIndex != -1) {
            if (timeoutNext != nullptr) {
                timeoutNext->timeoutPrev = timeoutPrev;
            }
            if (timeoutPrev != nullptr) {
                timeoutPrev->timeoutNext = timeoutNext;
            } else {
                ec->wheel[timeoutIndex] = timeoutNext;
            }
        }

        // 检查是否传入间隔时间
        if (interval) {
            // 如果设置了新的超时时间, 则放入相应的链表
            // 安全检查
            if (interval < 0 || interval > (int) ec->wheel.size()) throw std::logic_error("interval out of range");

            // 环形定位到 wheel 元素目标链表下标
            timeoutIndex = (interval + ec->cursor) & ((int) ec->wheel.size() - 1);

            // 成为链表头
            timeoutPrev = nullptr;
            timeoutNext = ec->wheel[timeoutIndex];
            ec->wheel[timeoutIndex] = this;

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
    }

    inline void Timer::SetTimeoutSeconds(double const &seconds) {
        SetTimeout(ec->SecondsToFrames(seconds));
    }

    inline Timer::~Timer() {
        if (timeoutIndex != -1) {
            SetTimeout(0);
        }
    }

    /***********************************************************************************************************/
    // TcpPeer
    inline void TcpPeer::Timeout() {
        Close(__LINE__);
    }

    inline void TcpPeer::EpollEvent(uint32_t const &e) {
        // error
        if (e & EPOLLERR || e & EPOLLHUP) {
            Close(__LINE__);
            return;
        }
        // read
        if (e & EPOLLIN) {
            // 如果接收缓存没容量就扩容( 通常发生在首次使用时 )
            if (!recv.cap) {
                recv.Reserve(1024 * 256);
            }
            // 如果数据长度 == buf限长 就自杀( 未处理数据累计太多? )
            if (recv.len == recv.cap) {
                Close(__LINE__);
                return;
            }
            // 通过 fd 从系统网络缓冲区读取数据. 追加填充到 recv 后面区域. 返回填充长度. <= 0 则认为失败 断开
            auto &&len = read(fd, recv.buf + recv.len, recv.cap - recv.len);
            if (len <= 0) {
                Close(__LINE__);
                return;
            }
            recv.len += len;

            // 调用用户数据处理函数
            Receive();
            if(!Alive()) return;
        }
        // write
        if (e & EPOLLOUT) {
            // 设置为可写状态
            writing = false;
            if (int r = Write()) {
                Close(__LINE__);
                return;
            }
        }
    }

    inline int TcpPeer::Send(Data &&data) {
        sendQueue.Push(std::move(data));
        return !writing ? Write() : 0;
    }

    inline int TcpPeer::Write() {
        // 如果没有待发送数据，停止监控 EPOLLOUT 并退出
        if (!sendQueue.bytes) return ec->Ctl(fd, EPOLLIN, EPOLL_CTL_MOD);

        // 前置准备
        // buf + len 数组指针
        std::array<iovec, UIO_MAXIOV> vs;
        // 数组长度
        int vsLen = 0;
        // 本次预计发送最大字节数
        size_t bufLen = 1024 * 256;

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
        return ec->Ctl(fd, EPOLLIN | EPOLLOUT, EPOLL_CTL_MOD);
    }

    // 和另一个 peer 互换 fd 和 mapping
    inline void TcpPeer::SwapFD(std::shared_ptr<TcpPeer> const& o) {
        if (!o || fd == o->fd) return;
        if (fd == -1) {
            ec->fdMappings[o->fd] = this;
            fd = o->fd;
            o->fd = -1;
        }
        else if(o->fd == -1) {
            ec->fdMappings[fd] = &*o;
            o->fd = fd;
            fd = -1;
        }
        else {
            std::swap(ec->fdMappings[fd], ec->fdMappings[o->fd]);
            std::swap(fd, o->fd);
        }
    }


    /***********************************************************************************************************/
    // TcpListener
    template<typename PeerType, class ENABLED>
    inline int TcpListener<PeerType, ENABLED>::Listen(int const& port) {
        // 先清一把
        Close(0);
        // 创建监听用 socket fd
        auto &&fd = ec->MakeSocketFD(port);
        if (fd < 0) return -1;
        // 确保 return 时自动 close
        xx::ScopeGuard sg([&] { close(fd); });
        // 开始监听
        if (-1 == listen(fd, SOMAXCONN)) return -2;
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
    inline void TcpListener<PeerType, ENABLED>::EpollEvent(uint32_t const &e) {
        // error
        if (e & EPOLLERR || e & EPOLLHUP) {
            Close(__LINE__);
            return;
        }
        // accept 到 没有 或 出错 为止
        while (HandleAccept(fd) > 0) {};
    }

    template<typename PeerType, class ENABLED>
    inline int TcpListener<PeerType, ENABLED>::HandleAccept(int const &listenFD) {
        // 开始创建 fd
        sockaddr_in6 addr;
        socklen_t len = sizeof(addr);
        // 接收并得到目标 fd
        int fd = accept(listenFD, (sockaddr *) &addr, &len);
        if (-1 == fd) {
            if (errno == EAGAIN || errno == EWOULDBLOCK) return 0;
            else return -1;
        }
        // 确保退出时自动关闭 fd
        ScopeGuard sg([&] { close(fd); });
        // 如果 fd 超出最大存储限制就退出。返回 fd 是为了外部能继续执行 accept
        if (fd >= (int) ec->fdMappings.size()) return fd;
        assert(!ec->fdMappings[fd]);
        // 设置非阻塞状态
        if (-1 == fcntl(fd, F_SETFL, fcntl(fd, F_GETFL, 0) | O_NONBLOCK)) return -2;
        // 设置一些 tcp 参数( 可选 )
        int on = 1;
        if (-1 == setsockopt(fd, IPPROTO_TCP, TCP_NODELAY, (const char *) &on, sizeof(on))) return -3;
        // 纳入 ep 管理
        if (-1 == ec->Ctl(fd, EPOLLIN)) return -4;
        // 撤销 "确保退出时自动关闭 fd"
        sg.Cancel();
        // 创建类容器
        auto&& p = xx::Make<PeerType>(ec, fd);
        // 填充 ip
        memcpy(&p->addr, &addr, len);
        // 触发事件
        Accept(p);
        // 返回这个便于 while 判断是否继续
        return fd;
    }


    /***********************************************************************************************************/
    // TcpConn
    template<typename PeerType, class ENABLED>
    inline void TcpConn<PeerType, ENABLED>::EpollEvent(uint32_t const &e) {
        // 如果 error 则 Close
        if (e & EPOLLERR || e & EPOLLHUP) {
            Close(__LINE__);
            return;
        }
        // 读取错误 或者读到错误 都认为是连接失败. 返回非 0 触发 Close
        int err;
        socklen_t result_len = sizeof(err);
        if (getsockopt(fd, SOL_SOCKET, SO_ERROR, &err, &result_len) == -1 || err) {
            Close(__LINE__);
            return;
        }

        // 连接成功. 自杀前备份变量到栈, 设置 fd 为 -1 避免被类析构 close. 清除映射关系 以便映射新的 处理类
        auto fd = this->fd;
        auto ec = this->ec;
        auto dialer = this->dialer;
        this->fd = -1;
        ec->fdMappings[fd] = nullptr;

        // 将导致 conn 都被析构, 即 this 上下文不复存在
        dialer->Stop();
        // 这之后只能用 "栈"变量

        // 创建具体 peer
        auto&& p = xx::Make<PeerType>(ec, fd);
        // fill address
        result_len = sizeof(p->addr);
        getpeername(fd, (sockaddr *) &p->addr, &result_len);
        dialer->Connect(p);
    }

    /***********************************************************************************************************/
    // TcpDialer
    template<typename PeerType, class ENABLED>
    inline int TcpDialer<PeerType, ENABLED>::AddAddress(std::string const &ip, int const &port) {
        auto &&a = addrs.emplace_back();
        if (int r = FillAddress(ip, port, a)) {
            addrs.pop_back();
            return r;
        }
        return 0;
    }

    template<typename PeerType, class ENABLED>
    inline int TcpDialer<PeerType, ENABLED>::Dial(int const &timeoutFrames) {
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

    template<typename PeerType, class ENABLED>
    int TcpDialer<PeerType, ENABLED>::DialSeconds(double const &timeoutSeconds) {
        return Dial(ec->SecondsToFrames(timeoutSeconds));
    }

    template<typename PeerType, class ENABLED>
    inline bool TcpDialer<PeerType, ENABLED>::Busy() {
        // 用超时检测判断是否正在拨号
        return timeoutIndex != -1;
    }

    template<typename PeerType, class ENABLED>
    inline void TcpDialer<PeerType, ENABLED>::Stop() {
        // 清理残留
        conns.clear();
        // 清除超时检测
        SetTimeout(0);
    }

    template<typename PeerType, class ENABLED>
    inline void TcpDialer<PeerType, ENABLED>::Timeout() {
        Stop();
        Connect(emptyPeer);
    }

    template<typename PeerType, class ENABLED>
    inline int TcpDialer<PeerType, ENABLED>::NewTcpConn(sockaddr_in6 const &addr) {
        // 创建 tcp 非阻塞 socket fd
        auto &&fd = socket(AF_INET, SOCK_STREAM | SOCK_NONBLOCK, 0);
        if (fd == -1) return -1;

        // 确保 return 时自动 close
        xx::ScopeGuard sg([&] { close(fd); });
        // 检测 fd 存储上限
        if (fd >= (int) ec->fdMappings.size()) return -2;
        assert(!ec->fdMappings[fd]);
        // 设置一些 tcp 参数( 可选 )
        int on = 1;
        if (-1 == setsockopt(fd, IPPROTO_TCP, TCP_NODELAY, (const char *) &on, sizeof(on))) return -3;
        // 开始连接
        if (connect(fd, (sockaddr *) &addr, sizeof(addr)) == -1) {
            if (errno != EINPROGRESS) return -4;
        }
        // else : 立刻连接上了
        // 纳入 epoll 管理
        if (int r = ec->Ctl(fd, EPOLLIN | EPOLLOUT)) return r;
        // 撤销 "自动 close"
        sg.Cancel();
        // 试创建目标类实例
        auto&& o = xx::Make<TcpConn<PeerType>>(ec, fd);
        // 继续初始化并放入容器
        o->dialer = xx::As<TcpDialer<PeerType, ENABLED>>(shared_from_this());
        conns.emplace_back(o);
        return 0;
    }


    /***********************************************************************************************************/
    // PipeReader
    inline void PipeReader::EpollEvent(uint32_t const &e) {
        char buf[512];
        if (read(fd, buf, sizeof(buf)) <= 0) return;
        ec->HandleActions();
    }

    /***********************************************************************************************************/
    // CommandHandler
    inline CommandHandler::CommandHandler(std::shared_ptr<Context> const& ec) : Item(ec, STDIN_FILENO) {
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
            auto &&iter = ec->cmds.find(args[0]);
            if (iter != ec->cmds.end()) {
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
            for (auto &&kv : self->ec->cmds) {
                auto &&word = kv.first;
                if (word.size() >= s.size() &&
                    word.compare(0, s.size(), s) == 0) {
                    matches.emplace_back(word);
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

    inline void CommandHandler::EpollEvent(uint32_t const &e) {
        // error
        if (e & EPOLLERR || e & EPOLLHUP) return;
        rl_callback_read_char();
    }

    inline CommandHandler::~CommandHandler() {
        // 清理 readline 的上下文
        rl_callback_handler_remove();
        rl_clear_history();

        // 这个 fd 特殊, 不 close. 只是解绑解控
        epoll_ctl(ec->efd, EPOLL_CTL_DEL, fd, nullptr);
        ec->fdMappings[fd] = nullptr;
        fd = -1;
    }


    /***********************************************************************************************************/
    // Context
    inline Context::Context(size_t const &wheelLen) {
        // 创建 epoll fd
        efd = epoll_create1(0);
        if (-1 == efd) throw std::logic_error("create epoll context failed.");
        // 初始化时间伦
        wheel.resize(wheelLen);
        // 初始化处理类映射表
        fdMappings.fill(nullptr);
    }

    inline Context::~Context() {
        // 关闭 epoll 本身 fd
        if (efd != -1) {
            close(efd);
            efd = -1;
        }
    }

    inline int Context::EnableCommandLine() {
        // 已存在：直接短路返回
        if (CommandHandler::self) return __LINE__;
        // 试将 fd 纳入 epoll 管理
        if (-1 == Ctl(STDIN_FILENO, EPOLLIN)) return __LINE__;
        // 创建单例
        xx::MakeTo(CommandHandler::self, shared_from_this());
        return 0;
    }

    inline void Context::DisableCommandLine() {
        if (CommandHandler::self && &*CommandHandler::self->ec == this) {
            CommandHandler::self.reset();
        }
    }

    inline int Context::MakeSocketFD(int const &port, int const &sockType) {
        char portStr[20];
        snprintf(portStr, sizeof(portStr), "%d", port);

        addrinfo hints;
        memset(&hints, 0, sizeof(addrinfo));
        hints.ai_family = AF_UNSPEC;                                            // ipv4 / 6
        hints.ai_socktype = sockType;                                           // SOCK_STREAM / SOCK_DGRAM
        hints.ai_flags = AI_PASSIVE;                                            // all interfaces

        addrinfo *ai_, *ai;
        if (getaddrinfo(nullptr, portStr, &hints, &ai_)) return __LINE__;

        int fd;
        for (ai = ai_; ai != nullptr; ai = ai->ai_next) {
            fd = socket(ai->ai_family, ai->ai_socktype | SOCK_NONBLOCK, ai->ai_protocol);
            if (fd == -1) continue;

            int enable = 1;
            if (setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, &enable, sizeof(int)) < 0) {
                close(fd);
                continue;
            }
            if (!bind(fd, ai->ai_addr, ai->ai_addrlen)) break;                  // success

            close(fd);
        }
        freeaddrinfo(ai_);
        if (!ai) return __LINE__;

        // 检测 fd 存储上限
        if (fd >= (int) fdMappings.size()) {
            close(fd);
            return __LINE__;
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
            h->EpollEvent(e);
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

            p->Timeout();
            p = np;
        };
    }

    inline int Context::HandleActions() {
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

    inline int Context::SetFrameRate(const double &frameRate_) {
        // 参数检查
        if (frameRate_ <= 0) return __LINE__;
        // 保存帧率
        frameRate = frameRate_;
        // 计算帧时间间隔
        ticksPerFrame = 10000000.0 / frameRate_;
        return 0;
    }

    inline int Context::Run() {
        // 创建 pipe fd. 失败返回编号
        int actionsPipes[2];
        if (int r = pipe(actionsPipes)) return __LINE__;
        // 设置 pipe 为非阻塞
        fcntl(actionsPipes[1], F_SETFL, O_NONBLOCK);
        fcntl(actionsPipes[0], F_SETFL, O_NONBLOCK);
        // 将 pipe fd 纳入 epoll 管理
        Ctl(actionsPipes[0], EPOLLIN);
        // 为 pipe fd[0] 创建容器
        xx::MakeTo(pipeReader, shared_from_this(), actionsPipes[0]);
        xx::MakeTo(pipeWriter, shared_from_this(), actionsPipes[1]);

        // 函数退出时自动删掉 pipe 容器, 以避免 Context 的引用计数无法清 0
        xx::ScopeGuard sg([&]{
            pipeReader.reset();
            pipeWriter.reset();
        });

        // 稳定帧回调用的时间池
        double ticksPool = 0;
        // 本次要 Wait 的超时时长
        int waitMS = 0;
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
                // 驱动 timer
                UpdateTimeoutWheel();
                // 帧逻辑调用一次
                if (int r = FrameUpdate()) return r;
            } else {
                // 计算等待时长
                waitMS = (int) ((ticksPerFrame - elapsedTicks) / 10000.0);
            }
            // 调用一次 epoll wait.
            if (int r = Wait(waitMS)) return r;
            // 清除延迟杀死的 items
            if (!deadItems.empty()) {
                for (auto &&item : deadItems) {
                    holdItems.erase(item);
                }
                deadItems.clear();
            }
            // 执行延迟函数
            if (!delayFuncs.empty()) {
                for (auto &&func : delayFuncs) {
                    func();
                }
                delayFuncs.clear();
            }
        }
        return 0;
    }

    inline int Context::Dispatch(std::function<void()>&& action) {
        {
            // 锁定并塞函数到队列
            std::scoped_lock<std::mutex> g(actionsMutex);
            actions.emplace_back(std::move(action));
        }
        return pipeWriter->WriteSome();
    }
    inline void Context::DelayExecute(std::function<void()>&& func) {
        delayFuncs.emplace_back(std::move(action));
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

    inline std::string AddressToString(sockaddr *const &in) {
        if (in) {
            char hbuf[NI_MAXHOST], sbuf[NI_MAXSERV];
            if (!getnameinfo(in, in->sa_family == AF_INET6 ? INET6_ADDRSTRLEN : INET_ADDRSTRLEN
                    , hbuf, sizeof hbuf
                    , sbuf, sizeof sbuf, NI_NUMERICHOST | NI_NUMERICSERV)) {
                return std::string(hbuf) + ":" + sbuf;
            }
        }
        return "";
    }
    inline std::string AddressToString(sockaddr_in6 const &in) {
        return AddressToString((sockaddr *) &in);
    }
}
