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
struct Service : EP::Context {

	// 在构造函数中根据 config 进一步初始化
	Service(size_t const& wheelLen = 1 << 12);

	// 帧逻辑放在这里
	virtual int FrameUpdate() override;

	// 析构当前类中的一些成员
	~Service();

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

	// 收到数据的处理
	virtual void OnReceive() override;

};







inline EP::TcpPeer_u Listener::OnCreatePeer() {
	// 返回 Peer 类实例
	return xx::TryMakeU<Peer>();
}

inline void Listener::OnAccept(EP::TcpPeer_r const& peer_) {
	// 还原为本来的类型
	auto&& peer = peer_.As<Peer>();

	// 设置 5 秒后断线
	peer->SetTimeoutSeconds(5);
}

inline void Peer::OnReceive() {
	/*
	// todo: 支持 json
	开头均以这个打头
	{ "__name__":"", ...........
	*/

	// echo
	if (Send(recv.buf, recv.len)) {
		OnDisconnect(-3);
		Dispose();
	}
	else {
		recv.Clear();
	}
}


inline Service::Service(size_t const& wheelLen) : EP::Context(wheelLen) {

	this->listener = CreateTcpListener<Listener>(::config.listenPort);
	if (!this->listener) {
		throw - 1;
	}
}

inline int Service::FrameUpdate() {
	// todo
	std::cout << ".";
	std::cout.flush();
	return 0;
}

inline Service::~Service() {
	// todo
}
