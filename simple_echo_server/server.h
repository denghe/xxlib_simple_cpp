#pragma once
#include "xx_epoll.h"
#include "config.h"

namespace EP = xx::Epoll;

// 预声明
struct Listener;
using Listener_r = EP::Ref<Listener>;

struct Peer;
using Peer_r = EP::Ref<Peer>;


// 服务本体
struct Server : EP::Context {
	// 在构造函数中根据 config 进一步初始化
	Server(size_t const& wheelLen = 1 << 12);

	// 帧逻辑放在这里
	virtual int FrameUpdate() override;

	// 析构当前类中的一些成员
	~Server();

	// 监听器
	Listener_r listener;
};




// 继承默认监听器覆盖关键函数
struct Listener : EP::TcpListener {
	// 透传构造函数
	using EP::TcpListener::TcpListener;

	// 提供创建目标类实例的内存操作支持
	virtual EP::TcpPeer_u OnCreatePeer() override;

	// 连接已建立, 搞事
	virtual void OnAccept(EP::TcpPeer_r const& peer_) override;
};


// 继承 默认 连接覆盖收包函数
struct Peer : EP::TcpPeer {
	// 处理接收事件
	void HandleReceive(char* buf, size_t len);

	// 实现收数据逻辑
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


inline EP::TcpPeer_u Listener::OnCreatePeer() {
	// 返回 Peer 类实例
	return xx::TryMakeU<Peer>();
}

inline void Listener::OnAccept(EP::TcpPeer_r const& peer_) {
	// 还原为本来的类型
	auto&& peer = peer_.As<Peer>();

	// 设置 n 秒后断线
	peer->SetTimeoutSeconds(30);

	std::cout << "client(" << EP::AddressToString(peer->addr) << ") connected." << std::endl;
}




inline void Peer::OnDisconnect(int const& reason) {
	std::cout << EP::AddressToString(addr) << " disconnected. reason = " << reason << std::endl;
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

    // 包头容器
    Header h;

    // 数据偏移
	size_t offset = 0;

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
	std::cout << "recv message from client(" << EP::AddressToString(addr) << "): " << std::string_view(buf, len) << std::endl;

	// todo: 内容处理

	// 先实现 echo
	SendPackage(buf, len);

	// 如果内容合法, 则重新设置 n 秒后断线( keep alive 的效果 )
	SetTimeoutSeconds(30);
}




inline Server::Server(size_t const& wheelLen) : EP::Context(wheelLen) {
	// 按配置的端口创建监听器
	listener = CreateTcpListener<Listener>(::config.listenPort);
	if (!listener) {
        throw std::logic_error(std::string("listen to port: ") + std::to_string(config.listenPort) + " failed.");
	}

	// todo: 注册交互指令?
}

inline int Server::FrameUpdate() {
	// todo
	//std::cout << ".";
	//std::cout.flush();
	return 0;
}

inline Server::~Server() {
	// todo
}
