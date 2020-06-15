#pragma once
#include "xx_epoll.h"

namespace EP = xx::Epoll;

// 继承 默认 连接覆盖收包函数
struct Peer : EP::TcpPeer {
    // 透传构造函数
    using EP::TcpPeer::TcpPeer;

	// 处理接收事件
	void ReceivePackage(char* buf, size_t len);

	// 实现收数据逻辑
	void Receive() override;

	// 断线事件( 清除自持有 )
	bool Close(int const& reason) override;

	// 在数据前面附带上 长度 并发送. 返回 非0 表示出状况( 但不一定是断线 )
	int SendPackage(char const* const& buf, size_t const& len);
};
