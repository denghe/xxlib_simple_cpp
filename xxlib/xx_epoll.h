#pragma once
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <netdb.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <signal.h>
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

#include "xx_ptr.h"
#include "xx_chrono.h"
#include "xx_data_queue.h"
#include "xx_itempool.h"
#include "xx_scopeguard.h"



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
		Context* ep = nullptr;

		// linux 系统文件描述符. 用不上就保持默认值
		int fd = -1;

		// 留个拿到依赖填充完整后的初始化口子( 比如启动 timer, 发首包啥的 )
		inline virtual void Init() {};

		// epoll 事件处理. 用不上不必实现
		inline virtual void OnEpollEvent(uint32_t const& e) {}

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
	template<typename T>
	struct Ref {
		ItemPool<Item_u>* items = nullptr;
		int index = -1;
		int64_t version = 0;

		// 会从 ptr 中提取 ep & indexAtContainer. 故需要保证这些值有效
		Ref(T* const& ptr);
		Ref(std::unique_ptr<T> const& ptr);

		Ref() = default;
		Ref(Ref const&) = default;
		Ref(Ref&&);
		Ref& operator=(Ref const&) = default;
		Ref& operator=(Ref&&);

		template<typename U>
		Ref& operator=(std::enable_if_t<std::is_base_of_v<T, U>, Ref<U>> const& o);
		template<typename U>
		Ref& operator=(std::enable_if_t<std::is_base_of_v<T, U>, Ref<U>>&& o);

		template<typename U>
		Ref<U> As() const;

		operator bool() const;
		T* operator->() const;
		T* Lock() const;
		template<typename U = T>
		void Reset(U* const& ptr = nullptr);
	};

	template<typename A, typename B>
	inline bool operator==(Ref<A> const& a, Ref<B> const& b) {
		return a.Lock() == b.Lock();
	}

	template<typename A, typename B>
	inline bool operator!=(Ref<A> const& a, Ref<B> const& b) {
		return a.Lock() != b.Lock();
	}

	using Item_r = Ref<Item>;


	/***********************************************************************************************************/
	// ItemTimeout
	/***********************************************************************************************************/

	// 需要自带超时功能的 item 可继承
	struct ItemTimeout : Item {
		int timeoutIndex = -1;
		ItemTimeout* timeoutPrev = nullptr;
		ItemTimeout* timeoutNext = nullptr;
		int SetTimeout(int const& interval);
		int SetTimeoutSeconds(double const& seconds);
		virtual void OnTimeout() = 0;
		~ItemTimeout();
	};


	/***********************************************************************************************************/
	// Timer
	/***********************************************************************************************************/

	struct Timer;
	using Timer_r = Ref<Timer>;
	struct Timer : ItemTimeout {
		// 时间到达时触发. 如果想实现 repeat 效果, 就在函数返回前 自己 timer->SetTimeout
		std::function<void(Timer_r const& timer)> onFire;

		// 超时触发 onFire + 可选 Dispose
		virtual void OnTimeout() override;
	};


	/***********************************************************************************************************/
	// Peer
	/***********************************************************************************************************/

	struct Peer : ItemTimeout {
		// 对方的 addr( udp 收到数据时就会刷新这个属性. Send 将采用这个属性 )
		sockaddr_in6 addr;

		// 收数据用堆积容器
		Data recv;

		// 读缓冲区内存扩容增量
		size_t readBufLen = 65536;


		// 数据接收事件: 从 recv 拿数据. 默认实现为 echo
		virtual void OnReceive();

		// 断线事件. 默认实现为 空
		virtual void OnDisconnect(int const& reason);

		// buf + len 塞队列并开始发送
		virtual int Send(char const* const& buf, size_t const& len) = 0;

		// Data 对象移进队列并开始发送( 通常来自序列化器 CutData. 也可以 xx::Data(buf, len) 临时构造 )
		virtual int Send(Data&& data) = 0;

		// 立刻开始发送数据
		virtual int Flush() = 0;

	protected:
		// 超时触发 Disconnect(-4) + Dispose
		virtual void OnTimeout() override;
	};

	using Peer_r = Ref<Peer>;
	using Peer_u = std::unique_ptr<Peer>;



	/***********************************************************************************************************/
	// TcpPeer
	/***********************************************************************************************************/

	struct TcpPeer : Peer {

		// 是否正在发送( 是：不管 sendQueue 空不空，都不能 write, 只能塞 sendQueue )
		bool writing = false;

		// 待发送队列
		xx::DataQueue sendQueue;

		// 每 fd 每一次可写, 写入的长度限制( 希望能实现当大量数据下发时各个 socket 公平的占用带宽 )
		size_t sendLenPerFrame = 65536;

		virtual void OnEpollEvent(uint32_t const& e) override;
		virtual int Send(Data&& data) override;
		virtual int Send(char const* const& buf, size_t const& len) override;
		virtual int Flush() override;
	protected:
		int Write();
	};

	using TcpPeer_r = Ref<TcpPeer>;
	using TcpPeer_u = std::unique_ptr<TcpPeer>;



	/***********************************************************************************************************/
	// TcpListener
	/***********************************************************************************************************/

	struct TcpListener : Item {
		typedef TcpPeer PeerType;
		// 提供创建 peer 对象的实现
		virtual TcpPeer_u OnCreatePeer();

		// 提供为 peer 绑定事件的实现
		inline virtual void OnAccept(TcpPeer_r const& peer) {}

		// 调用 accept
		virtual void OnEpollEvent(uint32_t const& e) override;
	protected:
		// return fd. <0: error. 0: empty (EAGAIN / EWOULDBLOCK), > 0: fd
		int Accept(int const& listenFD);
	};



	/***********************************************************************************************************/
	// TcpConn
	/***********************************************************************************************************/

	struct Dialer;
	using Dialer_r = Ref<Dialer>;
	struct TcpConn : Item {
		// 指向拨号器, 方便调用其 OnConnect 函数
		Dialer_r dialer;

		// 判断是否连接成功
		virtual void OnEpollEvent(uint32_t const& e) override;
	};
	using TcpConn_r = Ref<TcpConn>;



	/***********************************************************************************************************/
	// Dialer
	/***********************************************************************************************************/

	struct Dialer : ItemTimeout {
		// 要连的地址数组. 带协议标记
		std::vector<sockaddr_in6> addrs;

		// 向 addrs 追加地址. 如果地址转换错误将返回非 0
		int AddAddress(std::string const& ip, int const& port);

		// 开始拨号。会遍历 addrs 为每个地址创建一个 ?cpConn 连接
		// 保留先连接上的 socket fd, 创建 Peer 并触发 OnConnect 事件. 
		// 如果超时，也触发 OnConnect，参数为 nullptr
		int Dial(int const& timeoutFrames);

		// 返回是否正在拨号
		bool Busy();

		// 停止拨号 并清理 conns. 保留 addrs.
		void Stop();

		// 连接成功或超时后触发
		virtual void OnConnect(Peer_r const& peer) = 0;

		// 覆盖并提供创建 peer 对象的实现. 返回 nullptr 表示创建失败
		virtual Peer_u OnCreatePeer();

		// Stop()
		~Dialer();

		// 存个空值备用 以方便返回引用
		Peer_r emptyPeer;

		// 内部连接对象. 拨号完毕后会被清空
		std::vector<Item_r> conns;

		// 超时表明所有连接都没有连上. 触发 OnConnect( nullptr )
		virtual void OnTimeout() override;
	protected:
		// 按具体协议创建 Conn 对象
		int NewTcpConn(sockaddr_in6 const& addr);
		int NewKcpConn(sockaddr_in6 const& addr);
	};


	/***********************************************************************************************************/
	// CommandHandler
	/***********************************************************************************************************/

	// 处理键盘输入指令的专用类( 单例 ). 直接映射到 STDIN_FILENO ( fd == 0 )
	struct CommandHandler : Item {
		inline static CommandHandler* self = nullptr;
		std::vector<std::string> args;

		CommandHandler();
		static void ReadLineCallback(char* line);
		static char** CompleteCallback(const char* text, int start, int end);
		static char* CompleteGenerate(const char* text, int state);
		virtual void OnEpollEvent(uint32_t const& e) override;
		virtual ~CommandHandler();

	protected:
		// 解析 row 内容并调用 cmd 绑定 handler
		void Exec(char const* const& row, size_t const& len);
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
		std::array<Item*, 40000> fdMappings;

		// epoll_wait 事件存储
		std::array<epoll_event, 4096> events;

		// 存储的 epoll fd
		int efd = -1;


		// 时间轮. 只存指针引用, 不管理内存
		std::vector<ItemTimeout*> wheel;

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
		std::unordered_map<std::string, std::function<void(std::vector<std::string> const& args)>> cmds;


		// 执行标志位。如果要退出，修改它
		bool running = true;




		// 参数：时间轮长度( 要求为 2^n )
		Context(size_t const& wheelLen = 1 << 12);

		virtual ~Context();

		Context(Context const&) = delete;
		Context& operator=(Context const&) = delete;


		// 创建非阻塞 socket fd 并返回. < 0: error
		int MakeSocketFD(int const& port, int const& sockType = SOCK_STREAM); // SOCK_DGRAM

		// 添加 fd 到 epoll 监视. return !0: error
		int Ctl(int const& fd, uint32_t const& flags, int const& op = EPOLL_CTL_ADD);

		// 关闭并从 epoll 移除监视
		int CloseDel(int const& fd);

		// 进入一次 epoll wait. 可传入超时时间. 
		int Wait(int const& timeoutMS);


		// 每帧调用一次 以驱动 timer
		inline void UpdateTimeoutWheel();

		// 添加对象到容器
		template<typename T>
		T* AddItem(std::unique_ptr<T>&& item, int const& fd = -1);



		/********************************************************/
		// 下面是外部主要使用的函数

		// 将秒转为帧数
		inline int SecToFrames(double const& sec) { return (int)(frameRate * sec); }

		// 将毫秒转为帧数
		inline int MsToFrames(int const& ms) { return (int)(frameRate * ms / 1000); }


		// 帧逻辑可以覆盖这个函数. 返回非 0 将令 Run 退出. 
		inline virtual int FrameUpdate() { return 0; }


		// 开始运行并尽量维持在指定帧率. 临时拖慢将补帧
		int Run(double const& frameRate = 10);


		// 创建 TCP 监听器
		template<typename T = TcpListener, typename ...Args>
		Ref<T> CreateTcpListener(int const& port, Args&&... args);

		// 创建 拨号器
		template<typename T = Dialer, typename ...Args>
		Ref<T> CreateDialer(Args&&... args);

		// 创建 定时器
		template<typename T = Timer, typename ...Args>
		Ref<T> CreateTimer(int const& interval, std::function<void(Timer_r const& timer)>&& cb, Args&&...args);


		// 启用命令行输入控制. 支持方向键, tab 补齐, 上下历史
		CommandHandler* EnableCommandLine();
	};



	/***********************************************************************************************************/
	// Util funcs
	/***********************************************************************************************************/

	// ip, port 转为 addr
	int FillAddress(std::string const& ip, int const& port, sockaddr_in6& addr);

	
	//int AddressToString
	//namespace xx {
	//	// 适配 sockaddr*
	//	template<typename T>
	//	struct SFuncs<T, std::enable_if_t<std::is_same_v<sockaddr*, std::decay_t<T>>>> {
	//		static inline void Append(std::string& s, T const& in) noexcept {
	//			char hbuf[NI_MAXHOST], sbuf[NI_MAXSERV];
	//			if (!getnameinfo(in, in->sa_family == AF_INET6 ? INET6_ADDRSTRLEN : INET_ADDRSTRLEN
	//				, hbuf, sizeof hbuf, sbuf, sizeof sbuf, NI_NUMERICHOST | NI_NUMERICSERV)) {
	//				xx::Append(s, (char*)hbuf, ":", (char*)sbuf);
	//			}
	//		}
	//	};
	//	template<typename T>
	//	struct SFuncs<T, std::enable_if_t<std::is_same_v<sockaddr_in, std::decay_t<T>> || std::is_same_v<sockaddr_in6, std::decay_t<T>>>> {
	//		static inline void Append(std::string& s, T const& in) noexcept {
	//			return SFuncs<sockaddr*, void>::Append(s, (sockaddr*)&in);
	//		}
	//	};
	//}
}


#include "xx_epoll.hpp"
