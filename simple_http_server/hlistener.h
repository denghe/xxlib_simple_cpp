#pragma once
#include "xx_epoll.h"
#include "hpeer.h"

namespace EP = xx::Epoll;

// 继承默认监听器覆盖关键函数
struct HListener : EP::TcpListener<HPeer> {
    // 透传构造函数
	using EP::TcpListener<HPeer>::TcpListener;

	// 连接已建立, 搞事
	void Accept(std::shared_ptr<HPeer> const& peer) override;
};
