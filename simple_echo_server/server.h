#pragma once
#include "xx_epoll_sp.h"
#include "config.h"

namespace EP = xx::Epoll;

// 预声明
struct Listener;
struct Peer;

// 服务本体
struct Server : EP::Context {
    // 透传构造函数
    using EP::Context::Context;

	// 帧逻辑放在这里
	int FrameUpdate() override;

	// 监听器
	std::shared_ptr<Listener> listener;

	// run 之前初始化
	std::string Init();

	// run 之后擦屁股
	void Dispose();
};



// 继承默认监听器覆盖关键函数
struct Listener : EP::TcpListener<Peer> {
    // 透传构造函数
	using EP::TcpListener<Peer>::TcpListener;

	// 连接已建立, 搞事
	void OnAccept(std::shared_ptr<Peer> const& peer) override;
};


// 继承 默认 连接覆盖收包函数
struct Peer : EP::TcpPeer {
    // 透传构造函数
    using EP::TcpPeer::TcpPeer;

	// 处理接收事件
	void HandleReceive(char* buf, size_t len);

	// 实现收数据逻辑
	void OnReceive() override;

	// 断线事件( 清除自持有 )
	void OnDisconnect(int const& reason) override;

	// 在数据前面附带上 长度 并发送. 返回 非0 表示出状况( 但不一定是断线 )
	int SendPackage(char const* const& buf, size_t const& len);
};


// 包头. 包结构简单的定为  header(len) + data
struct Header {
	uint16_t len;
};
