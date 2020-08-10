#pragma once

#include <memory>
#include <exception>

namespace xx {

    struct Looper;

    struct Timer : std::enable_shared_from_this<Timer> {
    protected:
        friend Looper;

        // 位于时间轮的下标
        int timeoutIndex = -1;
        // 指向上一个对象（在时间轮每一格形成一个双链表）
        Timer *timeoutPrev = nullptr;
        // 指向下一个对象（在时间轮每一格形成一个双链表）
        Timer *timeoutNext = nullptr;

        // 指向循环器
        std::shared_ptr<Looper> looper;

    public:
        // 传入循环器并持有
        explicit Timer(std::shared_ptr<Looper> const &looper);

        // SetTimeout(0)
        virtual ~Timer();

        // 设置超时( 单位：帧 )
        void SetTimeout(int const &interval);

        // 设置超时( 单位：秒 )
        void SetTimeoutSeconds(double const &seconds);

        // Close
        virtual void Timeout();

        // 将当前实例的智能指针放入 looper->holdItems( 不能在构造函数或析构中执行 )
        void Hold();

        // 将当前实例的指针放入 looper->deadItems( 不能在析构中执行 ) 稍后会从 looper->holdItems 移除以触发析构
        void DelayUnhold();

        // user func
        virtual bool Close(int const &reason, char const *const &desc) = 0;
    };

    struct Looper : std::enable_shared_from_this<Timer> {
        /*************************************************************************/
        // public
        /*************************************************************************/
        // 执行标志位。如果要退出，修改它
        bool running = true;

        // 公共只读: 每帧开始时更新一下
        int64_t nowMS = 0;

        // Run 时填充, 以便于局部获取并转换时间单位
        double frameRate = 10;

        // 帧时间间隔
        double ticksPerFrame = 10000000.0 / frameRate;

        /*************************************************************************/
        // protected
        /*************************************************************************/
        // item 的智能指针的保持容器
        std::unordered_map<Timer *, std::shared_ptr<Timer>> holdItems;

        // 要删除一个 peer 就把它的 指针 压到这个队列. 会在 稍后 从 items 删除
        std::vector<Timer *> deadItems;

        // 时间轮. 只存指针引用, 不管理内存
        std::vector<Timer *> wheel;

        // 指向时间轮的游标
        int cursor = 0;

        // 参数：时间轮长度( 要求为 2^n. 如果帧率很高 300? 500+? 该值可能需要改大 )
        explicit Looper(size_t const &wheelLen = (1u << 12u));

        Looper(Looper const &) = delete;

        Looper &operator=(Looper const &) = delete;

        virtual ~Looper() = default;

        // 帧逻辑可以覆盖这个函数. 返回非 0 将令 Run 退出
        inline virtual int FrameUpdate() { return 0; }

        // 将秒转为帧数
        inline int SecondsToFrames(double const &sec) const { return (int) (frameRate * sec); }

        // 初始化 Run 帧率
        virtual int SetFrameRate(double const &frameRate);

        // 每帧调用一次 以驱动 timer
        inline void UpdateTimeoutWheel();

        // 开始循环
        inline int Run();

        // 提供帧间睡眠功能( 可覆盖放别的, 例如 epoll_wait ). 返回非 0 会导致 Run 退出
        inline virtual int Wait(int const& ms);
    };


    /*************************************************************************/
    // Timer impls
    /*************************************************************************/

    inline Timer::Timer(std::shared_ptr<Looper> const &looper)
            : looper(looper) {
    }

    inline void Timer::SetTimeout(int const &interval) {
        // 试着从 wheel 链表中移除
        if (timeoutIndex != -1) {
            if (timeoutNext != nullptr) {
                timeoutNext->timeoutPrev = timeoutPrev;
            }
            if (timeoutPrev != nullptr) {
                timeoutPrev->timeoutNext = timeoutNext;
            } else {
                looper->wheel[timeoutIndex] = timeoutNext;
            }
        }

        // 检查是否传入间隔时间
        if (interval) {
            // 如果设置了新的超时时间, 则放入相应的链表
            // 安全检查
            if (interval < 0 || interval > (int) looper->wheel.size())
                throw std::logic_error(
                        __LINESTR__
                        " Timer SetTimeout if (interval < 0 || interval > (int) looper->wheel.size())");

            // 环形定位到 wheel 元素目标链表下标
            timeoutIndex = (interval + looper->cursor) & ((int) looper->wheel.size() - 1);

            // 成为链表头
            timeoutPrev = nullptr;
            timeoutNext = looper->wheel[timeoutIndex];
            looper->wheel[timeoutIndex] = this;

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
        SetTimeout(looper->SecondsToFrames(seconds));
    }

    inline Timer::~Timer() {
        SetTimeout(0);
    }

    inline void Timer::Timeout() {
        Close(-1, __LINESTR__ " Timer Timeout");
    }

    inline void Timer::Hold() {
        looper->holdItems[this] = shared_from_this();
    }

    inline void Timer::DelayUnhold() {
        looper->deadItems.emplace_back(this);
    }

    /*************************************************************************/
    // Looper impls
    /*************************************************************************/

    inline Looper::Looper(size_t const &wheelLen) {
        // 初始化时间伦
        wheel.resize(wheelLen);
    }

    inline int Looper::SetFrameRate(double const &frameRate_) {
        // 参数检查
        if (frameRate_ <= 0) return -1;
        // 保存帧率
        frameRate = frameRate_;
        // 计算帧时间间隔
        ticksPerFrame = 10000000.0 / frameRate_;
        return 0;
    }

    inline void Looper::UpdateTimeoutWheel() {
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

    inline int Looper::Run() {
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
        }
        return 0;
    }

    inline int Looper::Wait(int const& ms) {
        std::this_thread::sleep_for(std::chrono::milliseconds(ms));
        return 0;
    }
}
