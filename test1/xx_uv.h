#pragma once

#include <mutex>
#include <deque>
#include <utility>
#include <thread>
#include <iostream>
#include "uv.h"
#include "xx_data.h"
#include "ikcp.h"
#include "xx_scopeguard.h"
#include "xx_chrono.h"

namespace xx::Uv {
    struct Context;

    // 所有受 Context 管理的子类的基类
    struct Item : std::enable_shared_from_this<Item> {
        // 指向总容器
        std::shared_ptr<Context> uc;

        // 初始化依赖上下文
        explicit Item(std::shared_ptr<Context> const &uc);

        // 注意：析构中调用虚函数，不会 call 到派生类的 override 版本
        virtual ~Item() = default;

        // 将当前实例的智能指针放入 ec->holdItems( 不能在构造函数或析构中执行 )
        void Hold();

        // 将当前实例的指针放入 ec->deadItems( 不能在析构中执行 ) 稍后会从 ec->holdItems 移除以触发析构
        void DelayUnhold();
    };

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

    // 域名解析器
    struct Resolver : Timer {
        // 在 uv 所需正常请求包内存后方附加 this 指针方便在回调中使用
        struct uv_getaddrinfo_t_ex : uv_getaddrinfo_t {
            Resolver *thiz;
        };
        // 类生命周期需确保大于 req 的
        uv_getaddrinfo_t_ex *req = nullptr;
        // 查询提示
        addrinfo hints{};
        // 域名解析的结果
        std::vector<std::string> ips;

        // 继承构造函数
        using Timer::Timer;

        // 析构时也删一把，保险
        ~Resolver() override { Stop(); }

        // 撤销解析. 不会执行 Finish
        void Stop();

        bool Busy() const { return req != nullptr; }

        int Resolve(std::string const &domainName, double const &timeoutSeconds = 5);

        // 解析完成时会被调用. code 非0 则有问题
        inline virtual void Finish(int const &code) {};

        // 供 uv 回调的静态函数. 通过 req 转为 ex 版找回 this
        static void UvCallback(uv_getaddrinfo_t *req_, int status, struct addrinfo *ai);

        // Stop + Finish(__LINE__)
        void Timeout() override;
    };

    struct Peer : Timer {
        xx::Data recv;

        using Timer::Timer;

        inline void Timeout() override { Close(__LINE__); }

        virtual int Init() = 0;

        inline virtual void Update() {}

        virtual bool Close(int const &reason) = 0;

        virtual bool Closed() const = 0;

        virtual bool Alive() const = 0;

        inline virtual bool IsKcp() const { return false; };

        virtual std::string GetIP() const = 0;

        virtual int Send(char const *const &buf, size_t const &len) = 0;

        inline virtual void Flush() {}

        virtual void Receive() = 0;
    };

    struct uv_write_t_ex : uv_write_t {
        uv_buf_t buf;
    };

    struct TcpPeer : Peer {
        struct uv_tcp_t_ex : uv_tcp_t {
            TcpPeer *thiz;
        };
        uv_tcp_t_ex *uvTcp = nullptr;
        std::string ip;

        using Peer::Peer;
        ~TcpPeer() override { Close(0); }

        int Init() override;

        bool Close(int const &reason) override;

        bool Closed() const override;

        bool Alive() const override;

        bool IsKcp() const override;

        std::string GetIP() const override;

        int Send(char const *const &buf, size_t const &len) override;
        //void Flush() override
        //void Receive() override;
    };

    template<typename PeerType, class ENABLED = std::is_base_of<TcpPeer, PeerType>>
    struct TcpListener : Item {
        struct uv_tcp_t_ex : uv_tcp_t {
            TcpListener *thiz;
        };
        uv_tcp_t_ex *uvTcp = nullptr;

        using Item::Item;

        int Listen(std::string const &ip, int const &port, int const &backlog = 128);

        virtual bool Alive();

        virtual bool Closed();

        virtual int Close(int const &reason);

        virtual void Accept(std::shared_ptr<PeerType> peer) = 0;
    };

    template<typename PeerType, class ENABLED = std::is_base_of<TcpPeer, PeerType>>
    struct TcpDialer;

    template<typename PeerType, class ENABLED = std::is_base_of<TcpPeer, PeerType>>
    struct TcpConn : Item {
        struct uv_connect_t_ex {
            uv_connect_t req;
            TcpConn *thiz;
        };
        uv_connect_t_ex* uvConn = nullptr;

        // 连接上下文
        decltype(PeerType::uvTcp) uvTcp = nullptr;

        // 指向拨号器, 方便调用其 Connect 函数
        std::shared_ptr<TcpDialer<PeerType>> dialer;

        // 根据地址初始化连接上下文和连接对象
        int Init(std::shared_ptr<TcpDialer<PeerType>> const& dialer, sockaddr_in6 const& addr);
        void Close();
        // todo: 转让 uvTcp 变量给 PeerType

        static void Callback(uv_connect_t *conn, int status);
    };

    template<typename PeerType, class ENABLED>
    struct TcpDialer : Timer {
        // 要连的地址数组. 带协议标记
        std::vector<sockaddr_in6> addrs;
        // 存个空值备用 以方便返回引用
        std::shared_ptr<PeerType> emptyPeer;
        // 内部连接对象. 拨号完毕后会被清空
        std::vector<std::shared_ptr<TcpConn<PeerType>>> conns;

        using Timer::Timer;
        ~TcpDialer() override { Stop(); }

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

        // 停止拨号 并清理 reqs. 保留 addrs.
        void Stop();

        // 超时表明所有连接都没有连上. 触发 Connect( nullptr )
        void Timeout() override;

        // 连接成功或超时后触发
        virtual void Connect(std::shared_ptr<PeerType> const &peer) = 0;
    };

    struct Context : std::enable_shared_from_this<Context> {
        // 执行标志位。如果要退出，修改它
        bool running = true;
        // 公共只读: 每帧开始时更新一下
        int64_t nowMS = 0;
        // Run 时填充, 以便于局部获取并转换时间单位
        double frameRate = 1;
        // 公用 buf( 需要的地方可临时用用 )
        std::array<char, 256 * 1024> buf{};
        // 公用 data( 需要的地方可临时用用 )
        xx::Data data;

        // 时间轮. 只存指针引用, 不管理内存
        std::vector<Timer *> wheel;
        // 指向时间轮的游标
        int cursor = 0;
        // item 的智能指针的保持容器
        std::unordered_map<Item *, std::shared_ptr<Item>> holdItems;
        // 要删除一个 peer 就把它的 指针 压到这个队列. 会在 稍后 从 items 删除
        std::vector<Item *> deadItems;
        // 延迟到 wait 之后执行的函数集合
        std::vector<std::function<void()>> delayFuncs;

        uint32_t maxPackageLength = 1024 * 256;
        uv_run_mode runMode = UV_RUN_DEFAULT;
        uv_loop_t uvLoop{};

        // 参数：时间轮长度( 要求为 2^n )
        explicit Context(size_t const &wheelLen = (1u << 12u));

        Context(Context const &) = delete;

        Context &operator=(Context const &) = delete;

        virtual ~Context();

        // 通知 uv_run 停止
        void Stop();

        // 帧逻辑可以覆盖这个函数. 返回非 0 将令 Run 退出
        inline virtual int FrameUpdate() { return 0; }

        // 开始运行并尽量维持在指定帧率. 临时拖慢将补帧
        virtual int Run(double const &frameRate);

        // 每帧调用一次 以驱动 timer
        void UpdateTimeoutWheel();

        // 将秒转为帧数
        [[nodiscard]] inline int SecondsToFrames(double const &sec) const { return (int) (frameRate * sec); }

        template<typename T>
        static void HandleCloseAndFree(T *&tar);

        static void AllocCB(uv_handle_t *h, std::size_t suggested_size, uv_buf_t *buf);

        static int FillIP(sockaddr_in6 &saddr, std::string &ip, bool includePort = true);

        static int FillIP(uv_tcp_t *stream, std::string &ip, bool includePort = true);

        static int FillAddress(std::string const &ip, int const &port, sockaddr_in6 &addr);
    };


    inline Item::Item(std::shared_ptr<Context> const &uc) : uc(uc) {}

    inline void Item::Hold() {
        uc->holdItems[this] = shared_from_this();
    }

    inline void Item::DelayUnhold() {
        uc->deadItems.emplace_back(this);
    }

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
                uc->wheel[timeoutIndex] = timeoutNext;
            }
        }

        // 检查是否传入间隔时间
        if (interval) {
            // 如果设置了新的超时时间, 则放入相应的链表
            // 安全检查
            if (interval < 0 || interval > (int) uc->wheel.size()) throw std::logic_error("interval out of range");

            // 环形定位到 wheel 元素目标链表下标
            timeoutIndex = (interval + uc->cursor) & ((int) uc->wheel.size() - 1);

            // 成为链表头
            timeoutPrev = nullptr;
            timeoutNext = uc->wheel[timeoutIndex];
            uc->wheel[timeoutIndex] = this;

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
        SetTimeout(uc->SecondsToFrames(seconds));
    }

    inline Timer::~Timer() {
        if (timeoutIndex != -1) {
            SetTimeout(0);
        }
    }


    inline Context::Context(size_t const &wheelLen) {
        if (int r = uv_loop_init(&uvLoop))
            throw std::logic_error(std::string("uv_loop_init error. r = ") + std::to_string(r));
        // 初始化时间伦
        wheel.resize(wheelLen);
    }

    inline Context::~Context() {
        int r = uv_run(&uvLoop, UV_RUN_DEFAULT);
        assert(!r);
        r = uv_loop_close(&uvLoop);
        assert(!r);
        (void) r;
    }

    inline void Context::Stop() {
        uv_stop(&uvLoop);
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

    inline int Context::Run(double const &frameRate_) {
        // 参数检查
        if (frameRate_ <= 0) return __LINE__;
        // 保存帧率
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
                // 驱动 timer
                UpdateTimeoutWheel();
                // 帧逻辑调用一次
                if (int r = FrameUpdate()) {
                    std::cout << "FrameUpdate r = " << r << std::endl;
                    return r;
                }
            } else {
                // 计算等待时长
                waitMS = (int) ((ticksPerFrame - elapsedTicks) / 10000.0);
            }
            // 调用一次 uv_run
            int r = uv_run(&uvLoop, UV_RUN_NOWAIT);
            if (r != 0 && r != 1) {
                std::cout << "uv_run r = " << r << std::endl;
                return r;
            }
            // 小睡片刻
            std::this_thread::sleep_for(std::chrono::milliseconds(waitMS));
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

    template<typename T>
    inline void Context::HandleCloseAndFree(T *&tar) {
        if (!tar) return;
        auto h = (uv_handle_t *) tar;
        tar = nullptr;
        assert(!uv_is_closing(h));
        uv_close(h, [](uv_handle_t *handle) {
            free(handle);
        });
    }


    inline void Context::AllocCB(uv_handle_t *h, std::size_t suggested_size, uv_buf_t *buf) {
        buf->base = (char *) ::malloc(suggested_size);
        buf->len = decltype(uv_buf_t::len)(suggested_size);
    }

    inline int Context::FillIP(sockaddr_in6 &saddr, std::string &ip, bool includePort) {
        ip.resize(64);
        if (saddr.sin6_family == AF_INET6) {
            if (int r = uv_ip6_name(&saddr, (char *) ip.data(), ip.size())) return r;
            ip.resize(strlen(ip.data()));
            if (includePort) {
                ip.append(":");
                ip.append(std::to_string(ntohs(saddr.sin6_port)));
            }
        } else {
            if (int r = uv_ip4_name((sockaddr_in *) &saddr, (char *) ip.data(), ip.size())) return r;
            ip.resize(strlen(ip.data()));
            if (includePort) {
                ip.append(":");
                ip.append(std::to_string(ntohs(((sockaddr_in *) &saddr)->sin_port)));
            }
        }
        return 0;
    }

    inline int Context::FillIP(uv_tcp_t *stream, std::string &ip, bool includePort) {
        sockaddr_in6 saddr;
        int len = sizeof(saddr);
        int r = 0;
        if ((r = uv_tcp_getpeername(stream, (sockaddr *) &saddr, &len))) return r;
        return FillIP(saddr, ip, includePort);
    }

    inline int Context::FillAddress(std::string const &ip, int const &port, sockaddr_in6 &addr) {
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


    inline void Resolver::Timeout() {
        Stop();
        Finish(__LINE__);
    }

    inline void Resolver::Stop() {
        if (!req) return;
        ips.clear();
        // cancel 之后也会回调
        uv_cancel((uv_req_t *) req);
        // 解绑
        req->thiz = nullptr;
        // 清理
        req = nullptr;
        SetTimeout(0);
    }

    inline int Resolver::Resolve(std::string const &domainName, double const &timeoutSeconds) {
        // 正在执行
        if (req) return __LINE__;
#ifdef __IPHONE_OS_VERSION_MIN_REQUIRED
        hints.ai_family = PF_UNSPEC;
        hints.ai_socktype = SOCK_STREAM;
        hints.ai_protocol = 0;// IPPROTO_TCP;
        hints.ai_flags = AI_DEFAULT;
#endif
        SetTimeoutSeconds(timeoutSeconds);
        // 申请内存并关联. 在回调中释放内存并脱钩
        xx::MallocTo(req)->thiz = this;
        if (int r = uv_getaddrinfo(&uc->uvLoop, (uv_getaddrinfo_t *) req, UvCallback, domainName.c_str(), nullptr,
#ifdef __IPHONE_OS_VERSION_MIN_REQUIRED
                (const addrinfo*) & hints
#else
                                   nullptr
#endif
        )) {
            Stop();
            return r;
        }
        return 0;
    }

    inline void Resolver::UvCallback(uv_getaddrinfo_t *req_, int status, struct addrinfo *ai) {
        // 得到原始数据类型
        auto &&req = (uv_getaddrinfo_t_ex *) req_;
        // 设定 return 时 自动释放内存
        xx::ScopeGuard sg([&] {
            free(req);
            if (ai) {
                uv_freeaddrinfo(ai);
            }
        });
        // 定位到 this
        auto thiz = req->thiz;
        // cleanup 以便再次发起
        if (thiz) {
            thiz->req = nullptr;
            thiz->SetTimeout(0);
        }
            // 如果已 cancel( status == -4081 )则退出
        else return;
        // 出错: 回调并返回
        if (status) {
            thiz->Finish(__LINE__);
            return;
        }
        assert(ai);
        // 准备容器
        auto &ips = thiz->ips;
        ips.clear();
        // 读出地址转为 string 存到容器
        std::string s;
        do {
            s.resize(64);
            if (ai->ai_addr->sa_family == AF_INET6) {
                uv_ip6_name((sockaddr_in6 *) ai->ai_addr, (char *) s.data(),
                            s.size());
            } else {
                uv_ip4_name((sockaddr_in *) ai->ai_addr, (char *) s.data(),
                            s.size());
            }
            s.resize(strlen(s.data()));

            // 为 ios & android 的返回结果去重
            if (std::find(ips.begin(), ips.end(), s) == ips.end()) {
                ips.push_back(std::move(s));
            }
            ai = ai->ai_next;
        } while (ai);
        // 产生成功回调
        thiz->Finish(0);
    }


    inline int TcpPeer::Init() {
        // 初始化分3个阶段. 1: uv_tcp_init     2: uv_accept( listener )     3: uv_read_start
        if (!uvTcp) {
            xx::MallocTo(uvTcp)->thiz = this;
            if (int r = uv_tcp_init(&uc->uvLoop, uvTcp)) {
                free(uvTcp);
                uvTcp = nullptr;
                return r;
            }
            return 0;
        }
        return uv_read_start((uv_stream_t *) uvTcp, Context::AllocCB,
                             [](uv_stream_t *stream, ssize_t nread, const uv_buf_t *buf) {
                                 // 拿到类实例指针
                                 auto thiz = (*(uv_tcp_t_ex *) (stream)).thiz;
                                 // 设置自动回收 buf
                                 xx::ScopeGuard sg([&] {
                                     if (buf) {
                                         free(buf->base);
                                     }
                                 });
                                 // 如果已解绑就退出
                                 if (!thiz) return;

                                 // 如果有数据则追加到 recv, 并调用接收函数
                                 if (nread > 0) {
                                     thiz->recv.WriteBuf((uint8_t *) buf->base, (uint32_t) nread);
                                     thiz->Receive();
                                     // 如果已解绑就退出( 可能 Receive() 过程中调用过 Close )
                                     if (!thiz) return;
                                 }
                                 // 如果出错则???  // todo: 是否还需要关闭?
                                 if (nread < 0) {
                                     thiz->Close(__LINE__);
                                     thiz = nullptr;                   // todo: 已关闭？不再需要关闭？
                                 }
                             });
    }

    inline bool TcpPeer::Close(int const &reason) {
        if (!uvTcp) return false;
        uvTcp->thiz = nullptr;
        uc->HandleCloseAndFree(uvTcp);
        DelayUnhold();
        return true;
    }

    inline bool TcpPeer::Closed() const { return !uvTcp; }

    inline bool TcpPeer::Alive() const { return uvTcp != nullptr; }

    inline bool TcpPeer::IsKcp() const { return false; }

    inline std::string TcpPeer::GetIP() const { return ip; }

    inline int TcpPeer::Send(char const *const &buf, size_t const &len) {
        // 在结构体内存块后面直接附带数据
        auto req = (uv_write_t_ex *) malloc(sizeof(uv_write_t_ex) + len);
        req->buf.base = (char *) req + sizeof(uv_write_t_ex);
        req->buf.len = decltype(uv_buf_t::len)(len);
        ::memcpy(req->buf.base, buf, len);
        // 立刻检测到发送失败就关闭并返回错误码
        if (int r = uv_write(req, (uv_stream_t *) uvTcp, &req->buf, 1, [](uv_write_t *req, int status) {
            free(req);
        })) {
            Close(__LINE__);
            return r;
        }
        return 0;
    }


    template<typename PeerType, class ENABLED>
    inline int TcpListener<PeerType, ENABLED>::Listen(std::string const &ip, int const &port, int const &backlog) {
        xx::MallocTo(uvTcp)->thiz = this;
        if (!uvTcp) return __LINE__;
        if (int r = uv_tcp_init(&uc->uvLoop, uvTcp)) {
            free(uvTcp);
            uvTcp = nullptr;
            return __LINE__;
        }

        // ipv4/6 check
        sockaddr_in6 addr;
        if (ip.find(':') == std::string::npos) {
            if (uv_ip4_addr(ip.c_str(), port, (sockaddr_in *) &addr)) return __LINE__;
        } else {
            if (uv_ip6_addr(ip.c_str(), port, &addr)) return __LINE__;
        }
        if (uv_tcp_bind(uvTcp, (sockaddr *) &addr, 0)) return __LINE__;

        if (uv_listen((uv_stream_t *) uvTcp, backlog, [](uv_stream_t *server, int status) {
            // 如果有问题直接退出( 没啥需要回收的 )
            if (status) return;
            // 拿到类实例指针
            auto thiz = (*(uv_tcp_t_ex *) (server)).thiz;
            // 如果已解绑就退出
            if (!thiz) return;

            // 创建 peer 并逐步初始化
            auto &&peer = xx::Make<PeerType>(thiz->uc);
            if (peer->Init()) return;
            if (uv_accept(server, (uv_stream_t *) peer->uvTcp)) return;
            if (peer->Init()) return;
            Context::FillIP(peer->uvTcp, peer->ip);

            // 产生回调
            thiz->Accept(peer);
        }))
            return __LINE__;
        return 0;
    };

    template<typename PeerType, class ENABLED>
    inline bool TcpListener<PeerType, ENABLED>::Alive() {
        return uvTcp != nullptr;
    }

    template<typename PeerType, class ENABLED>
    inline bool TcpListener<PeerType, ENABLED>::Closed() {
        return uvTcp == nullptr;
    }

    template<typename PeerType, class ENABLED>
    inline int TcpListener<PeerType, ENABLED>::Close(int const &reason) {
        if (!uvTcp) return __LINE__;
        uvTcp->thiz = nullptr;
        Context::HandleCloseAndFree(uvTcp);
        return 0;
    }



    template<typename PeerType, class ENABLED>
    int TcpConn<PeerType, ENABLED>::Init(std::shared_ptr<TcpDialer<PeerType>> const& dialer_, sockaddr_in6 const& addr) {
        if (int r = uv_tcp_init(&uc->uvLoop, uvTcp)) {
            free(uvTcp);
            uvTcp = nullptr;
            return r;
        }
        xx::MallocTo(uvTcp)->thiz = nullptr;
        xx::MallocTo(uvConn)->thiz = this;
        if (int r = uv_tcp_connect(uvConn, uvTcp, (sockaddr *) &addr, Callback)) {
            Context::HandleCloseAndFree(uvTcp);
            uvConn->thiz = nullptr;
            free(uvConn);
            uvConn = nullptr;
            return r;
        }

        // todo

        dialer = dialer_;
        return 0;
    }

    template<typename PeerType, class ENABLED>
    void TcpConn<PeerType, ENABLED>::Close() {
        if (!uvTcp) return;
        uvTcp->thiz = nullptr;
        uc->HandleCloseAndFree(uvTcp);
        // todo: close uvConn
        dialer.reset();
        DelayUnhold();
    }

    template<typename PeerType, class ENABLED>
    void TcpConn<PeerType, ENABLED>::Callback(uv_connect_t *conn, int status) {
        auto&& thiz = ((uv_connect_t_ex*)conn)->thiz;
        if (status) {
            if (!thiz) return;
            thiz->Close();
            return;
        }
        // todo: 将 uvTcp 移到新 peer

//       std::shared_ptr<UvTcpDialer> dialer;
//       std::shared_ptr<UvTcpPeerBase> peer;
//       {
//           // auto delete when exit scope
//           auto &&req = std::unique_ptr<uv_connect_t_ex>(container_of(conn, uv_connect_t_ex, req));
//           if (status) return;                                // error or -4081 canceled
//           if (!req->peer) return;                            // canceled
//           dialer = req->dialer_w.lock();
//           if (!dialer) return;                            // container disposed
//           peer = std::move(req->peer);                    // remove peer to outside, avoid cancel
//       }
//       if (peer->ReadStart()) return;                        // read error
//       Uv::FillIP(peer->uvTcp, peer->ip);
//       dialer->Accept(peer);                                // callback
    }


    template<typename PeerType, class ENABLED>
    int TcpDialer<PeerType, ENABLED>::AddAddress(std::string const &ip, int const &port) {
        auto &&a = addrs.emplace_back();
        if (int r = Context::FillAddress(ip, port, a)) {
            addrs.pop_back();
            return r;
        }
        return 0;
    }

    template<typename PeerType, class ENABLED>
    int TcpDialer<PeerType, ENABLED>::Dial(int const &timeoutFrames) {
        // todo
        for(auto&& addr : addrs) {

        }
        return 0;
    }

    template<typename PeerType, class ENABLED>
    int TcpDialer<PeerType, ENABLED>::DialSeconds(double const &timeoutSeconds) {
        return Dial(uc->SecondsToFrames(timeoutSeconds));
    }

    template<typename PeerType, class ENABLED>
    bool TcpDialer<PeerType, ENABLED>::Busy() {
        // 用超时检测判断是否正在拨号
        return timeoutIndex != -1;
    }

    template<typename PeerType, class ENABLED>
    void TcpDialer<PeerType, ENABLED>::Stop() {
        // 清理残留
        //conns.clear(); todo
        // 清除超时检测
        SetTimeout(0);
    }

    template<typename PeerType, class ENABLED>
    void TcpDialer<PeerType, ENABLED>::Timeout() {
        Stop();
        Connect(emptyPeer);
    }
}


//            sgReq.Cancel();
//            return 0;
//        }
//
//        inline virtual void Cancel() noexcept override {
//            if (disposed) return;
//            if (reqs.len) {
//                for (auto i = reqs.len - 1; i != (std::size_t) -1; --i) {
//                    auto req = reqs[i];
//                    assert(req->peer);
//                    uv_cancel((uv_req_t *) &req->req);                // ios call this do nothing
//                    if (reqs[i] == req) {                            // check req is alive
//                        req->peer.reset();                            // ios need this to fire cancel progress
//                    }
//                }
//            }
//            reqs.Clear();
//        }