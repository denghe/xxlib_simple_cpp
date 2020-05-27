#pragma once
#include "xx_epoll.h"

namespace EP = xx::Epoll;

// 预声明
struct Dialer;
using Dialer_r = EP::Ref<Dialer>;

struct Peer;
using Peer_r = EP::Ref<Peer>;


// 服务本体
struct Client : EP::Context {
	// 在构造函数中根据 config 进一步初始化
	Client(size_t const& wheelLen = 1 << 12);

	// 帧逻辑放在这里
	virtual int FrameUpdate() override;

	// 析构当前类中的一些成员
	~Client();

	// 拨号器
	Dialer_r dialer;

	// 拨号器连接成功后将 peer 存在此以便使用
	Peer_r peer;
};


// 继承默认监听器覆盖关键函数
struct Dialer : EP::TcpDialer {
	// 透传构造函数
	using EP::TcpDialer::TcpDialer;

	// 提供创建目标类实例的内存操作支持
	virtual EP::TcpPeer_u OnCreatePeer() override;

	// 连接已建立, 搞事
	virtual void OnConnect(EP::TcpPeer_r const& peer_) override;
};


// 继承 默认 连接覆盖收包函数
struct Peer : EP::TcpPeer {
	// 处理接收事件
	void HandleReceive(char* buf, size_t len);

	// 收到数据
	virtual void OnReceive() override;

	// 断线事件
	virtual void OnDisconnect(int const& reason) override;

	// 在数据前面附带上 长度 并发送. 返回 非0 表示出状况( 但不一定是断线 )
	int SendPackage(char const* const& buf, size_t const& len);
};


// 包头. 包结构简单的定为  header(len) + data
struct Header {
	uint16_t len;
};


inline EP::TcpPeer_u Dialer::OnCreatePeer() {
	// 返回 Peer 类实例
	return xx::TryMakeU<Peer>();
}

inline void Dialer::OnConnect(EP::TcpPeer_r const& peer_) {
	// 没连上
	if (!peer_) return;

	// 拿到 client 上下文
	auto c = (Client*)ep;

	// 将 peer 存放到 c 备用
	c->peer = peer_.As<Peer>();

	std::cout << "OnConnect" << std::endl;
}

inline void Peer::OnDisconnect(int const& reason) {
	std::cout << "OnDisconnect. reason = " << reason << std::endl;
}

inline int Peer::SendPackage(char const* const& buf, size_t const& len) {
	// 最大长度检测
	if (len > std::numeric_limits<decltype(Header::len)>::max()) return -9999;

	// 构造包头
	Header h;
	h.len = (decltype(Header::len))len;

	// 构造包数据容器并拼接
	xx::Data d(sizeof(h) + len);
	d.WriteBuf((char*)&h, sizeof(h));
	d.WriteBuf(buf, len);

	// 发送
	return Send(std::move(d));
}


inline void Peer::OnReceive() {
	// 死亡判断变量
	EP::Ref<Item> alive(this);

	// 数据偏移
	size_t offset = 0;

	// 包头容器
	Header h;

	// 确保包头长度充足
	while (offset + sizeof(h) <= recv.len) {

		// 拷贝数据头出来
		memcpy(&h, recv.buf + offset, sizeof(h));

		// 数据未接收完 就 跳出
		if (offset + sizeof(h) + h.len > recv.len) break;

		// 偏移量 指向 数据区
		offset += sizeof(h);

		// 调用处理函数
		HandleReceive(recv.buf + offset, h.len);

		// 如果当前类实例已自杀则退出
		if (!alive) return;

		// 跳到下一个包的开头
		offset += h.len;
	}

	// 移除掉已处理的数据( 将后面剩下的数据移动到头部 )
	recv.RemoveFront(offset);
}

inline void Peer::HandleReceive(char* buf, size_t len) {
	// 这里先输出收到的内容
	std::cout << "recv message from server: " << std::string_view(buf, len) << std::endl;

	// todo: 内容处理
}


inline Client::Client(size_t const& wheelLen) : EP::Context(wheelLen) {

	// 初始化拨号器
	this->dialer = CreateTcpDialer<Dialer>();
	if (!this->dialer) {
		throw - 1;
	}

	// 添加默认拨号地址
	dialer->AddAddress("127.0.0.1", 10000);

	// 注册交互指令
	this->EnableCommandLine();

	this->cmds["send"] = [this](std::vector<std::string> const& args) {
		if (args.size() < 2) {
			std::cout << "send: miss data" << std::endl;
		}
		else if (args[1].size() > std::numeric_limits<decltype(Header::len)>::max()) {
			std::cout << "send: data's size > max len" << std::endl;
		}
		else if (!this->peer) {
			std::cout << "send: no peer" << std::endl;
		}
		else {
			this->peer->SendPackage(args[1].data(), args[1].size());
			std::cout << "send: success" << std::endl;
		}
	};

	this->cmds["quit"] = [this](auto args) {
		this->running = false;
	};

	this->cmds["exit"] = this->cmds["quit"];
}

inline int Client::FrameUpdate() {
	// 自动拨号 & 重连逻辑
	// 如果未创建连接并且拨号器没有正在拨号, 就开始拨号
	if (!peer && !dialer->Busy()) {
		// 超时时间 2 秒
		dialer->DialSeconds(2);
	}

	//std::cout << ".";
	//std::cout.flush();
	return 0;
}

inline Client::~Client() {
	// todo
}
