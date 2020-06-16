#pragma once
#include "xx_epoll.h"
#include "peer.h"

namespace EP = xx::Epoll;

// 继承默认监听器覆盖关键函数
struct Listener : EP::TcpListener<Peer> {
    // 透传构造函数
	using EP::TcpListener<Peer>::TcpListener;

	// 连接已建立, 搞事
	void Accept(std::shared_ptr<Peer> const& peer) override;
};
