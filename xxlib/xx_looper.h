#pragma once
#include "xx_typehelpers.h"
#include "xx_chrono.h"
#include <memory>
#include <exception>
#include <functional>

namespace xx::Looper {

	// 总上下文。需确保它活得最久。换句话说，当它析构时，确保所有成员全部清掉
	struct Context;

	/*************************************************************************/
	// Timer
	/*************************************************************************/

	// Looper Context 成员基类
	struct Timer {
	protected:
		friend Context;

		// 位于时间轮的下标
		int timeoutIndex = -1;
		// 指向上一个对象（在时间轮每一格形成一个双链表）
		Timer* timeoutPrev = nullptr;
		// 指向下一个对象（在时间轮每一格形成一个双链表）
		Timer* timeoutNext = nullptr;

		// 指向循环器
		Context* ctx;

		// 试着从时间轮链表移除
		void RemoveFromWheel();
	public:
		explicit Timer(Context* const& ctx);

		// 清除超时
		virtual ~Timer();

		// 设置超时( 单位：秒 ). 如果时长小于 1 帧的间隔时长，则至少为 1 帧的间隔时长
		void SetTimeout(double const& seconds);

		// 清除超时
		void ClearTimeout();

		// Close
		virtual void Timeout() = 0;
	};

	/*************************************************************************/
	// TimerEx
	/*************************************************************************/

	// 可作为值类型成员使用的便捷版 timer
	// 小心：lambda 可能导致某些 shared_ptr 生命周期延长 或者令某些对象中途析构
	struct TimerEx : Timer {
		using Timer::Timer;
		std::function<void(TimerEx* const& self)> onTimeout;
		inline virtual void Timeout() { onTimeout(this); }
	};

	/*************************************************************************/
	// Item
	/*************************************************************************/

	// 业务逻辑常见基类
	// 注意：只能用 shared_ptr 包裹使用
	struct Item : Timer, std::enable_shared_from_this<Item> {
		// Timer(ctx)
		explicit Item(Context* const& ctx);

		// Close
		void Timeout() override;

		// 将当前实例的智能指针放入 ctx->holdItems( 不能在构造函数或析构中执行 )
		void Hold();

		// 将当前实例的指针放入 ctx->deadItems( 不能在析构中执行 ) 稍后会从 ctx->holdItems 移除以触发析构
		void DelayUnhold();

		// 需要派生类覆盖的函数
		virtual bool Close(int const& reason, char const* const& desc) = 0;

		// 需要派生类覆盖的函数( Close(0, ... ) )
		//~Item() override;
	};


	/*************************************************************************/
	// Context
	/*************************************************************************/

	struct Context {
		/************************************************/
		// public

		// 执行标志位。如果要退出，修改它
		bool running = true;

		// Run 时填充, 以便于局部获取并转换时间单位
		double frameRate = 10;

		// 帧时间间隔
		double secondsPerFrame = 1.0 / frameRate;

		// 当前帧编号
		int64_t frameNumber = 0;

		// 补帧时间池
		double secondsPool = 0;

		// 帧回调事件代码( 在 FrameUpdate 虚函数中调用. 也可覆盖该虚函数 )
		std::function<int(void)> onFrameUpdate;

		/************************************************/
		// protected

		// 指向时间轮的游标
		size_t cursor = 0;

		// 时间轮. 只存指针引用, 不管理内存
		std::vector<Timer*> wheel;

		// item 的智能指针的保持容器
		std::unordered_map<Item*, std::shared_ptr<Item>> holdItems;

		// 要删除一个 peer 就把它的 指针 压到这个队列. 会在 稍后 从 items 删除
		std::vector<Item*> deadItems;

		// 参数：时间轮长度( 要求为 2^n 对齐. 最大值 >= 最长超时时长 / 每帧耗时 )
		explicit Context(size_t const& wheelLen = (1u << 12u), double const& frameRate_ = 10);

		Context(Context const&) = delete;

		Context& operator=(Context const&) = delete;

		Context(Context&& o) noexcept;

		Context& operator=(Context&& o) noexcept;

		virtual ~Context() = default;

		// 帧逻辑可以覆盖这个函数. 返回非 0 将令 Run 退出
		inline virtual int FrameUpdate() { if (onFrameUpdate) return onFrameUpdate(); else return 0; }

		// 每帧调用一次 以驱动 timer
		void UpdateTimeoutWheel();

		// 调用前需先确保 经历时长入池. secondsPool += elapsedSeconds; 如果累计时长跨度大于一帧的时长 则 Update
		virtual int RunOnce();

		// 判断是否会发生 Update
		inline bool RunCheck() { return secondsPool >= secondsPerFrame; }
	};


	/*************************************************************************/
	// Timer impls
	/*************************************************************************/

	inline Timer::Timer(Context* const& ctx)
		: ctx(ctx) {
	}

	inline void Timer::RemoveFromWheel() {
		if (timeoutIndex != -1) {
			if (timeoutNext != nullptr) {
				timeoutNext->timeoutPrev = timeoutPrev;
			}
			if (timeoutPrev != nullptr) {
				timeoutPrev->timeoutNext = timeoutNext;
			}
			else {
				ctx->wheel[timeoutIndex] = timeoutNext;
			}
		}
	}

	inline void Timer::SetTimeout(double const& seconds) {
		// 如果有传入超时时长 则将当前对象挂到时间轮相应节点
		if (seconds != 0) {
			// 试着从 wheel 链表中移除
			RemoveFromWheel();

			// 计算帧间隔( 下标偏移 )
			auto interval = seconds < ctx->secondsPerFrame ? 1 : (size_t)(seconds / ctx->secondsPerFrame);

			// 帧间隔长度安全检查. 时间轮长度设置过小可能导致无法支持指定时长的超时行为
			if (interval > ctx->wheel.size()) {
				throw std::logic_error("Timer SetTimeout out of wheel.size() range");
			}

			// 环形定位到 wheel 元素目标链表下标
			timeoutIndex = (interval + ctx->cursor) & ((int)ctx->wheel.size() - 1);

			// 放入相应的链表, 成为链表头
			timeoutPrev = nullptr;
			timeoutNext = ctx->wheel[timeoutIndex];
			ctx->wheel[timeoutIndex] = this;

			// 和之前的链表头连起来( 如果有的话 )
			if (timeoutNext) {
				timeoutNext->timeoutPrev = this;
			}
		}
		else {
			ClearTimeout();
		}
	}

	inline void Timer::ClearTimeout() {
		if (timeoutIndex != -1) {
			RemoveFromWheel();
			timeoutIndex = -1;
			timeoutPrev = nullptr;
			timeoutNext = nullptr;
		}
	}

	inline Timer::~Timer() {
		RemoveFromWheel();
	}

	/*************************************************************************/
	// Item impls
	/*************************************************************************/

	inline Item::Item(Context* const& ctx) : Timer(ctx) {}

	inline void Item::Timeout() {
		Close(-1, "Timer Timeout");
	}

	inline void Item::Hold() {
		ctx->holdItems[this] = shared_from_this();
	}

	inline void Item::DelayUnhold() {
		ctx->deadItems.emplace_back(this);
	}

	/*************************************************************************/
	// Context impls
	/*************************************************************************/

	inline Context::Context(size_t const& wheelLen, double const& frameRate)
		: frameRate(frameRate)
		, secondsPerFrame(1.0 / frameRate)
	{
		// 初始化时间伦
		wheel.resize(wheelLen);
	}

	inline Context::Context(Context&& o) noexcept {
		this->operator=(std::move(o));
	}

	inline Context& Context::operator=(Context&& o) noexcept {
		std::swap(running, o.running);
		std::swap(frameRate, o.frameRate);
		std::swap(secondsPerFrame, o.secondsPerFrame);
		std::swap(frameNumber, o.frameNumber);
		std::swap(secondsPool, o.secondsPool);
		std::swap(cursor, o.cursor);
		std::swap(wheel, o.wheel);
		std::swap(holdItems, o.holdItems);
		std::swap(deadItems, o.deadItems);
		return *this;
	}

	inline void Context::UpdateTimeoutWheel() {
		cursor = (cursor + 1) & (wheel.size() - 1);
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

	inline int Context::RunOnce() {
		while (secondsPool >= secondsPerFrame) {
			// 消耗累计时长
			secondsPool -= secondsPerFrame;
			// 驱动 timer
			UpdateTimeoutWheel();
			// 帧逻辑调用一次
			if (int r = FrameUpdate()) return r;
			// 清除延迟杀死的 items
			if (!deadItems.empty()) {
				for (auto&& item : deadItems) {
					holdItems.erase(item);
				}
				deadItems.clear();
			}
			// 判断是否需要终止循环
			if (!running) return -1;
			// 步进帧编号
			++frameNumber;
		}
		return 0;
	}
}
