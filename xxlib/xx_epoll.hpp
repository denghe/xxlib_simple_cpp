#pragma once
#include "xx_epoll.h"

namespace xx::Epoll {

	/***********************************************************************************************************/
	// Item
	/***********************************************************************************************************/

	inline void Item::Dispose() {
		if (indexAtContainer != -1) {
			// 因为 gcc 傻逼，此处由于要自杀，故先复制参数到栈，避免出异常
			auto ep = this->ep;
			auto indexAtContainer = this->indexAtContainer;
			ep->items.RemoveAt(indexAtContainer);	// 触发析构
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
	inline Ref<T>::Ref(T* const& ptr) {
		static_assert(std::is_base_of_v<Item, T>);
		Reset(ptr);
	}

	template<typename T>
	inline Ref<T>::Ref(std::unique_ptr<T> const& ptr) : Ref(ptr.get()) {}

	template<typename T>
	inline Ref<T>::Ref(Ref&& o)
		: items(o.items)
		, index(o.index)
		, version(o.version) {
		o.items = nullptr;
		o.index = -1;
		o.version = 0;
	}

	template<typename T>
	Ref<T>& Ref<T>::operator=(Ref&& o) {
		std::swap(items, o.items);
		std::swap(index, o.index);
		std::swap(version, o.version);
		return *this;
	}


	template<typename T>
	template<typename U>
	Ref<T>& Ref<T>::operator=(std::enable_if_t<std::is_base_of_v<T, U>, Ref<U>> const& o) {
		return operator=(*(Ref<T>*) & o);
	}

	template<typename T>
	template<typename U>
	Ref<T>& Ref<T>::operator=(std::enable_if_t<std::is_base_of_v<T, U>, Ref<U>>&& o) {
		return operator=(std::move(*(Ref<T>*) & o));
	}


	template<typename T>
	template<typename U>
	inline Ref<U> Ref<T>::As() const {
		if (!dynamic_cast<U*>(Lock())) return Ref<U>();
		return *(Ref<U>*)this;
	}


	template<typename T>
	inline Ref<T>::operator bool() const {
		return version && items->VersionAt(index) == version;
	}

	template<typename T>
	inline T* Ref<T>::operator->() const {
		if (!operator bool()) throw - 1;		// 空指针
		return (T*)items->ValueAt(index).get();
	}

	template<typename T>
	inline T* Ref<T>::Lock() const {
		return operator bool() ? (T*)items->ValueAt(index).get() : nullptr;
	}

	template<typename T>
	template<typename U>
	inline void Ref<T>::Reset(U* const& ptr) {
		static_assert(std::is_base_of_v<T, U>);
		if (!ptr) {
			items = nullptr;
			index = -1;
			version = 0;
		}
		else {
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
	inline int ItemTimeout::SetTimeout(int const& interval) {
		assert((int)ep->wheel.size() > interval);

		// 试着从 wheel 链表中移除
		if (timeoutIndex != -1) {
			if (timeoutNext != nullptr) {
				timeoutNext->timeoutPrev = timeoutPrev;
			}
			if (timeoutPrev != nullptr) {
				timeoutPrev->timeoutNext = timeoutNext;
			}
			else {
				ep->wheel[timeoutIndex] = timeoutNext;
			}
		}

		// 检查是否传入间隔时间
		if (interval) {
			// 如果设置了新的超时时间, 则放入相应的链表
			// 安全检查
			if (interval < 0 || interval >(int)ep->wheel.size()) return -2;

			// 环形定位到 wheel 元素目标链表下标
			timeoutIndex = (interval + ep->cursor) & ((int)ep->wheel.size() - 1);

			// 成为链表头
			timeoutPrev = nullptr;
			timeoutNext = ep->wheel[timeoutIndex];
			ep->wheel[timeoutIndex] = this;

			// 和之前的链表头连起来( 如果有的话 )
			if (timeoutNext) {
				timeoutNext->timeoutPrev = this;
			}
		}
		else {
			// 重置到初始状态
			timeoutIndex = -1;
			timeoutPrev = nullptr;
			timeoutNext = nullptr;
		}

		return 0;
	}

	inline int ItemTimeout::SetTimeoutSeconds(double const& seconds) {
		return SetTimeout(ep->SecToFrames(seconds));
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
			Ref<Timer> alive(this);	// 防止在 onFire 中 Dispose timer
			onFire(this);
			if (!alive) return;
		}
		// 非 repeat 模式( 未再次 SetTimeout )直接自杀
		if (timeoutIndex == -1) {
			Dispose();
		}
	}


	/***********************************************************************************************************/
	// Peer
	/***********************************************************************************************************/

	inline void Peer::OnDisconnect(int const& reason) {}

	inline void Peer::OnReceive() {
		if (Send(recv.buf, recv.len)) {
			OnDisconnect(-3);
			Dispose();
		}
		else {
			recv.Clear();
		}
	}

	inline void Peer::OnTimeout() {
		Ref<Peer> alive(this);
		OnDisconnect(-4);
		if (alive) {
			Dispose();
		}
	}


	/***********************************************************************************************************/
	// TcpPeer
	/***********************************************************************************************************/

	inline void TcpPeer::OnEpollEvent(uint32_t const& e) {
		// error
		if (e & EPOLLERR || e & EPOLLHUP) {
			Ref<TcpPeer> alive(this);
			OnDisconnect(-1);
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
				OnDisconnect(-2);
				if (alive) {
					Dispose();
				}
				return;
			}

			// 通过 fd 从系统网络缓冲区读取数据. 追加填充到 recv 后面区域. 返回填充长度. <= 0 则认为失败 自杀
			auto&& len = read(fd, recv.buf + recv.len, recv.cap - recv.len);
			if (len <= 0) {
				Ref<TcpPeer> alive(this);
				OnDisconnect(-3);
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
				OnDisconnect(-4);
				if (alive) {
					Dispose();
				}
				return;
			}
		}
	}

	inline int TcpPeer::Send(char const* const& buf, size_t const& len) {
		sendQueue.Push(Data(buf, len));
		return !writing ? Write() : 0;
	}

	inline int TcpPeer::Send(Data&& data) {
		sendQueue.Push(std::move(data));
		return !writing ? Write() : 0;
	}

	inline int TcpPeer::Flush() {
		return !writing ? Write() : 0;
	}

	inline int TcpPeer::Write() {
		// 如果没有待发送数据，停止监控 EPOLLOUT 并退出
		if (!sendQueue.bytes) return ep->Ctl(fd, EPOLLIN, EPOLL_CTL_MOD);

		// 前置准备
		std::array<iovec, UIO_MAXIOV> vs;					// buf + len 数组指针
		int vsLen = 0;										// 数组长度
		auto bufLen = sendLenPerFrame;						// 计划发送字节数

		// 填充 vs, vsLen, bufLen 并返回预期 offset. 每次只发送 bufLen 长度
		auto&& offset = sendQueue.Fill(vs, vsLen, bufLen);

		// 返回值为 实际发出的字节数
		auto&& sentLen = writev(fd, vs.data(), vsLen);

		// 已断开
		if (sentLen == 0) return -2;

		// 繁忙 或 出错
		else if (sentLen == -1) {
			if (errno == EAGAIN) goto LabEnd;
			else return -3;
		}

		// 完整发送
		else if ((size_t)sentLen == bufLen) {
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

	inline void TcpListener::OnEpollEvent(uint32_t const& e) {
		// error
		if (e & EPOLLERR || e & EPOLLHUP) {
			Dispose();
			return;
		}
		// accept 到 没有 或 出错 为止
		while (Accept(fd) > 0) {};
	}

	inline int TcpListener::Accept(int const& listenFD) {
		// 开始创建 fd
		sockaddr_in6 addr;
		socklen_t len = sizeof(addr);

		// 接收并得到目标 fd
		int fd = accept(listenFD, (sockaddr*)&addr, &len);
		if (-1 == fd) {
			ep->lastErrorNumber = errno;
			if (ep->lastErrorNumber == EAGAIN || ep->lastErrorNumber == EWOULDBLOCK) return 0;
			else return -1;
		}

		// 确保退出时自动关闭 fd
		ScopeGuard sg([&] { close(fd); });

		// 如果 fd 超出最大存储限制就退出。返回 fd 是为了外部能继续执行 accept
		if (fd >= (int)ep->fdMappings.size()) return fd;
		assert(!ep->fdMappings[fd]);

		// 设置非阻塞状态
		if (-1 == fcntl(fd, F_SETFL, fcntl(fd, F_GETFL, 0) | O_NONBLOCK)) return -2;

		// 设置一些 tcp 参数( 可选 )
		int on = 1;
		if (-1 == setsockopt(fd, IPPROTO_TCP, TCP_NODELAY, (const char*)&on, sizeof(on))) return -3;

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

	inline void TcpConn::OnEpollEvent(uint32_t const& e) {
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
		Dispose();	// 自杀

		// 这之后只能用 "栈"变量
		d->Stop();
		auto peer = d->OnCreatePeer();
		if (peer) {
			auto p = ep->AddItem(std::move(peer), fd);	// peer is moved
			// fill address
			result_len = sizeof(p->addr);
			getpeername(fd, (sockaddr*)&p->addr, &result_len);
			d->OnConnect(p);
		}
		else {
			ep->CloseDel(fd);
			d->OnConnect(d->emptyPeer);
		}
	}



	/***********************************************************************************************************/
	// Dialer
	/***********************************************************************************************************/

	inline int Dialer::AddAddress(std::string const& ip, int const& port) {
		auto&& a = addrs.emplace_back();
		if (int r = FillAddress(ip, port, a)) {
			addrs.pop_back();
			return r;
		}
		return 0;
	}

	inline int Dialer::Dial(int const& timeoutFrames) {
		Stop();
		SetTimeout(timeoutFrames);
		for (auto&& a : addrs) {
			if (int r = NewTcpConn(a)) {
				Stop();
				return r;
			}
		}
		return 0;
	}

	inline bool Dialer::Busy() {
		// 用超时检测判断是否正在拨号
		return timeoutIndex != -1;
	}

	inline void Dialer::Stop() {
		// 清理原先的残留
		for (auto&& conn : conns) {
			if (auto&& c = conn.Lock()) {
				c->Dispose();
			}
		}
		conns.clear();

		// 清除超时检测
		SetTimeout(0);
	}

	inline void Dialer::OnTimeout() {
		Stop();
		OnConnect(emptyPeer);
	}

	inline Dialer::~Dialer() {
		Stop();
	}

	inline Peer_u Dialer::OnCreatePeer() {
		return xx::TryMakeU<TcpPeer>();
	}

	inline int Dialer::NewTcpConn(sockaddr_in6 const& addr) {
		// 创建 tcp 非阻塞 socket fd
		auto&& fd = socket(AF_INET, SOCK_STREAM | SOCK_NONBLOCK, 0);
		if (fd == -1) return -1;

		// 确保 return 时自动 close
		xx::ScopeGuard sg([&] { close(fd); });

		// 检测 fd 存储上限
		if (fd >= (int)ep->fdMappings.size()) return -2;
		assert(!ep->fdMappings[fd]);

		// 设置一些 tcp 参数( 可选 )
		int on = 1;
		if (-1 == setsockopt(fd, IPPROTO_TCP, TCP_NODELAY, (const char*)&on, sizeof(on))) return -3;

		// 开始连接
		if (connect(fd, (sockaddr*)&addr, sizeof(addr)) == -1) {
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
		conns.push_back(o);

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
		rl_attempted_completion_function = (rl_completion_func_t*)&CompleteCallback;
		rl_callback_handler_install("# ", (rl_vcpfunc_t*)&ReadLineCallback);
	}

	inline void CommandHandler::Exec(char const* const& row, size_t const& len) {
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
			}
			else if (c == '	' || c == ' '/* || c == 10*/) {
				args.emplace_back(std::move(s));
				jumpSpace = true;
				continue;
			}
			else {
				s += c;
			}
		}
		if (s.size()) {
			args.emplace_back(std::move(s));
		}

		if (args.size()) {
			auto&& iter = ep->cmds.find(args[0]);
			if (iter != ep->cmds.end()) {
				if (iter->second) {
					iter->second(args);
				}
			}
			else {
				std::cout << "unknown command: " << args[0] << std::endl;
			}
		}
	}

	inline void CommandHandler::ReadLineCallback(char* line) {
		if (!line) return;
		auto len = strlen(line);
		if (len) {
			add_history(line);
		}
		self->Exec(line, len);
		free(line);
	}

	inline char* CommandHandler::CompleteGenerate(const char* t, int state) {
		static std::vector<std::string> matches;
		static size_t match_index = 0;

		// 如果是首次回调 则开始填充对照表
		if (state == 0) {
			matches.clear();
			match_index = 0;

			std::string s(t);
			for (auto&& kv : self->ep->cmds) {
				auto&& word = kv.first;
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

	inline char** CommandHandler::CompleteCallback(const char* text, int start, int end) {
		// 不做文件名适配
		rl_attempted_completion_over = 1;
		return rl_completion_matches(text, (rl_compentry_func_t*)&CompleteGenerate);
	}

	inline void CommandHandler::OnEpollEvent(uint32_t const& e) {
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
	// Context
	/***********************************************************************************************************/

	inline Context::Context(size_t const& wheelLen) {
		// 创建 epoll fd
		efd = epoll_create1(0);
		if (-1 == efd) throw - 1;

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
	}

	inline int Context::MakeSocketFD(int const& port, int const& sockType) {
		char portStr[20];
		snprintf(portStr, sizeof(portStr), "%d", port);

		addrinfo hints;
		memset(&hints, 0, sizeof(addrinfo));
		hints.ai_family = AF_UNSPEC;										// ipv4 / 6
		hints.ai_socktype = sockType;										// SOCK_STREAM / SOCK_DGRAM
		hints.ai_flags = AI_PASSIVE;										// all interfaces

		addrinfo* ai_, * ai;
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
			if (!bind(fd, ai->ai_addr, ai->ai_addrlen)) break;				// success

			close(fd);
		}
		freeaddrinfo(ai_);

		if (!ai) return -2;

		// 检测 fd 存储上限
		if (fd >= (int)fdMappings.size()) {
			close(fd);
			return -3;
		}
		assert(!fdMappings[fd]);

		return fd;
	}

	inline int Context::Ctl(int const& fd, uint32_t const& flags, int const& op) {
		epoll_event event;
		bzero(&event, sizeof(event));
		event.data.fd = fd;
		event.events = flags;
		return epoll_ctl(efd, op, fd, &event);
	};

	inline int Context::CloseDel(int const& fd) {
		assert(fd != -1);
		epoll_ctl(efd, EPOLL_CTL_DEL, fd, nullptr);
		return close(fd);
	}

	inline int Context::Wait(int const& timeoutMS) {
		int n = epoll_wait(efd, events.data(), (int)events.size(), timeoutMS);
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
		cursor = (cursor + 1) & ((int)wheel.size() - 1);
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


	inline int Context::Run(double const& frameRate) {
		assert(frameRate > 0);
		this->frameRate = frameRate;

		// 稳定帧回调用的时间池
		double ticksPool = 0;

		// 本次要 Wait 的超时时长
		int waitMS = 0;

		// 计算帧时间间隔
		auto ticksPerFrame = 10000000.0 / frameRate;

		// 取当前时间
		auto lastTicks = xx::NowEpoch10m();

		// 更新一下逻辑可能用到的时间戳
		nowMS = xx::NowSteadyEpochMS();

		// 开始循环
		while (running) {

			// 计算上个循环到现在经历的时长, 并累加到 pool
			auto currTicks = xx::NowEpoch10m();
			auto elapsedTicks = (double)(currTicks - lastTicks);
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
			}
			else {
				// 计算等待时长
				waitMS = (int)((ticksPerFrame - elapsedTicks) / 10000.0);
			}

			// 调用一次 epoll wait. 
			if (int r = Wait(waitMS)) return r;
		}

		return 0;
	}


	template<typename L, typename ...Args>
	inline Ref<L> Context::CreateTcpListener(int const& port, Args&&... args) {
		static_assert(std::is_base_of_v<TcpListener, L>);

		// 创建监听用 socket fd
		auto&& fd = MakeSocketFD(port);
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
	inline Ref<TD> Context::CreateDialer(Args&&... args) {
		static_assert(std::is_base_of_v<Dialer, TD>);

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
	inline Ref<T> Context::CreateTimer(int const& interval, std::function<void(Timer_r const& timer)>&& cb, Args&&...args) {
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



	inline int FillAddress(std::string const& ip, int const& port, sockaddr_in6& addr) {
		memset(&addr, 0, sizeof(addr));

		if (ip.find(':') == std::string::npos) {		// ipv4
			auto a = (sockaddr_in*)&addr;
			a->sin_family = AF_INET;
			a->sin_port = htons((uint16_t)port);
			if (!inet_pton(AF_INET, ip.c_str(), &a->sin_addr)) return -1;
		}
		else {											// ipv6
			auto a = &addr;
			a->sin6_family = AF_INET6;
			a->sin6_port = htons((uint16_t)port);
			if (!inet_pton(AF_INET6, ip.c_str(), &a->sin6_addr)) return -1;
		}

		return 0;
	}


	template<typename T>
	inline T* Context::AddItem(std::unique_ptr<T>&& item, int const& fd) {
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




	inline CommandHandler* Context::EnableCommandLine() {
		// 已存在：直接短路返回
		if (fdMappings[STDIN_FILENO]) return (CommandHandler*)fdMappings[STDIN_FILENO];

		// 试将 fd 纳入 epoll 管理
		if (-1 == Ctl(STDIN_FILENO, EPOLLIN)) return nullptr;

		// 创建 stdin fd 的处理类并放入容器
		return AddItem(xx::MakeU<CommandHandler>(), STDIN_FILENO);
	}

}
