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
        explicit Item(std::shared_ptr<Context> uc);
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
            Resolver* resolver;
        };
        // 类生命周期需确保大于 req 的
        uv_getaddrinfo_t_ex* req = nullptr;
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
        inline virtual void Finish(int const& code) {};
        // 供 uv 回调的静态函数. 通过 req 转为 ex 版找回 this
        static void UvCallback(uv_getaddrinfo_t *req_, int status, struct addrinfo *ai);
        // Stop + Finish(__LINE__)
        void Timeout() override;
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
        std::vector<Timer*> wheel;
        // 指向时间轮的游标
        int cursor = 0;
        // item 的智能指针的保持容器
        std::unordered_map<Item*, std::shared_ptr<Item>> holdItems;
        // 要删除一个 peer 就把它的 指针 压到这个队列. 会在 稍后 从 items 删除
        std::vector<Item*> deadItems;
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
        // 帧逻辑可以覆盖这个函数. 返回非 0 将令 Run 退出
        inline virtual int FrameUpdate() { return 0; }
        // 开始运行并尽量维持在指定帧率. 临时拖慢将补帧
        virtual int Run(double const &frameRate);
        // 每帧调用一次 以驱动 timer
        void UpdateTimeoutWheel();
        // 将秒转为帧数
        [[nodiscard]] inline int SecondsToFrames(double const &sec) const { return (int) (frameRate * sec); }
    };










    inline Item::Item(std::shared_ptr<Context> uc)
            : uc(std::move(uc)) {
    }

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

//    inline void Context::Stop() {
//        uv_stop(&uvLoop);
//    }

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
        if(frameRate_ <= 0) return __LINE__;
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



    inline void Resolver::Timeout() {
        Stop();
        Finish(__LINE__);
    }

    inline void Resolver::Stop() {
        if (!req) return;
        ips.clear();
        // cancel 之后也会回调
        uv_cancel((uv_req_t*)req);
        // 解绑
        req->resolver = nullptr;
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
        xx::MallocTo(req)->resolver = this;
        if (int r = uv_getaddrinfo(&uc->uvLoop, (uv_getaddrinfo_t *)req, UvCallback, domainName.c_str(), nullptr,
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
        auto&& req = (uv_getaddrinfo_t_ex*)req_;
        // 设定 return 时 自动释放内存
        xx::ScopeGuard sg([&]{
            free(req);
            if (ai) {
                uv_freeaddrinfo(ai);
            }
        });
        // 定位到 this
        auto thiz = req->resolver;
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
}
