#pragma once
#include <asio.hpp>
#include <iostream>
#include <unordered_map>
#include <initializer_list>
#include <algorithm>
#include <string_view>
#include <queue>
#include "xx_looper.h"
#include "xx_data.h"
#include "ikcp.h"

// todo: 去掉 try 啥的

// 先为 cocos 之类 client 实现一个带 域名解析, kcp拨号 与 通信 的版本。不考虑性能

// 适配 ip 地址做 map key
namespace std {
	template <>
	struct hash<asio::ip::address> {
		size_t operator()(asio::ip::address const& v) const {
			if (v.is_v4()) return v.to_v4().to_ulong();
			else if (v.is_v6()) {
				auto bytes = v.to_v6().to_bytes();
				auto&& p = (uint64_t*)&bytes;
				return p[0] ^ p[1] ^ p[2] ^ p[3];
			}
			else return 0;
		}
	};
}

namespace xx::Asio {
	using namespace std::placeholders;

	struct KcpPeer;

	// 可域名解析，可拨号，最后访问其 peer 成员通信
	struct Client : xx::Looper::Context {
		using BaseType = xx::Looper::Context;

		explicit Client(size_t const& wheelLen = (1u << 12u), double const& frameRate_ = 60);

		Client(Client const&) = delete;
		Client& operator=(Client const&) = delete;

		// 被 cocos Update( delta ) 调用 以处理网络事件
		int RunOnce(double const& elapsedSeconds);

		// 附加每帧更新 kcp 处理逻辑
		int FrameUpdate() override;

		// 可临时存点啥
		void* userData = nullptr;
		size_t userSize = 0;
		int userInt = 0;

		// asio 上下文
		asio::io_context ioc;

		// 域名 -- 地址列表 映射
		std::unordered_map<std::string, std::vector<asio::ip::address>> domainAddrs;

		// 域名 -- 解析器 映射. 如果不空, 就是有正在解析的
		std::unordered_map<std::string, asio::ip::udp::resolver> domainResolvers;

		// 域名解析
		inline int ResolveDomain(std::string const& domain) {
			auto&& r = domainResolvers.emplace(domain, asio::ip::udp::resolver(ioc));
			if (r.second) {
				r.first->second.async_resolve(domain, "", [this, domain](const asio::error_code& error, asio::ip::udp::resolver::results_type results) {
					if (!error.value()) {
						auto&& as = domainAddrs[domain];
						as.clear();
						for (auto&& r : results) {
							as.push_back(r.endpoint().address());
						}
					}
					domainResolvers.erase(domain);
					});
				return 0;
			}
			// 域名正在解析中
			return -1;
		}

		// 打印解析出来的域名
		inline void DumpDomainAddrs() {
			for (auto&& kv : domainAddrs) {
				std::cout << "domain = \"" << kv.first << "\", ip list = {" << std::endl;
				for (auto&& a : kv.second) {
					std::cout << "    " << a << std::endl;
				}
				std::cout << "}" << std::endl;
			}
		}

		// 要 dial 的目标地址列表( 带端口 )
		std::unordered_map<asio::ip::address, std::vector<uint16_t>> dialAddrs;

		// 添加拨号地址. 端口合并去重
		inline void AddDialAddress(asio::ip::address const& a, std::initializer_list<uint16_t> ports_) {
			auto&& ps = dialAddrs[a];
			ps.insert(ps.end(), ports_);
			ps.erase(std::unique(ps.begin(), ps.end()), ps.end());
		}

		// 从 domainAddrs 拿域名对应的 ip 列表, 附加多个端口后放入拨号地址集合
		inline int AddDialDomain(std::string const& domain, std::initializer_list<uint16_t> ports_) {
			auto&& iter = domainAddrs.find(domain);
			if (iter == domainAddrs.end()) return -2;
			for (auto&& a : iter->second) {
				AddDialAddress(a, ports_);
			}
			return 0;
		}

		// 添加拨号地址( string 版 ). 端口合并去重
		inline void AddDialIP(std::string const& ip, std::initializer_list<uint16_t> ports_) {
			AddDialAddress(asio::ip::address::from_string(ip), ports_);
		}

		// 正在拨号的 peers 队列. 如果正在拨号，则该队列不空. 每 FrameUpdate 将驱动其逻辑
		std::vector<std::shared_ptr<KcpPeer>> dialPeers;

		// 握手用 timer
		xx::Looper::TimerEx shakeTimer;

		// 拨号超时检测用 timer
		xx::Looper::TimerEx dialTimer;

		// 自增以产生握手用序列号
		uint32_t shakeSerial = 0;

		// 根据 dialAddrs 的配置，对这些 ip:port 同时发起拨号( 带超时 ), 最先连上的存到 peer 并停止所有拨号
		void Dial(float const& timeoutSeconds);

		// 停止拨号
		void Stop();

		// 返回是否正在拨号
		inline bool Busy() { return !dialPeers.empty(); }

		// 当前 peer( 拨号成功将赋值 )
		std::shared_ptr<KcpPeer> peer;

		bool PeerAlive();
	};

	// 当前设计为 每个 peer 配一个 udp socket
	// 能握手，收到数据后放队列
	struct KcpPeer : xx::Looper::Item {
		using BaseType = xx::Looper::Item;

		// 关闭标志位
		int closed = 0;
		std::string closedDesc;

		// 收发用 socket
		asio::ip::udp::socket socket;

		// 对方的 ep( 似乎可以不用关心 )
		asio::ip::udp::endpoint ep;

		// udp 接收缓冲区
		char recvBuf[1024 * 64];

		// kcp 上下文
		ikcpcb* kcp = nullptr;

		// 服务器下发的 conv id( 每次收到 udp 包用这个来校验。如果对不上就忽略 )
		uint32_t conv = 0;

		// 握手时使用 
		uint32_t shakeSerial = 0;

		// 创建时的毫秒数( 
		int64_t createMS = 0;

		// kcp 接收缓冲区
		Data recv;

		// 已接收完整的数据队列( 从 recv 复制创建. 帧逻辑代码可 pop 出去处理 )
		std::deque<std::shared_ptr<Data>> recvs;

		// 初始化 sokcet
		explicit KcpPeer(Client* const& c, asio::ip::udp::endpoint const& ep, uint32_t const& shakeSerial)
			: BaseType(c)
			, socket(c->ioc, asio::ip::udp::endpoint(ep.protocol(), 0))
			, ep(ep)
			, shakeSerial(shakeSerial) {
		    socket.non_blocking(true);
			recv.Reserve(1024 * 256);
		}

		// 回收 kcp
		~KcpPeer() override {
			Close(-__LINE__, "~KcpPeer()");
		}

		// every frame call it
		inline void Recv() {
            if (closed) return;
            asio::error_code e;
            asio::ip::udp::endpoint p;
            if (auto recvLen = socket.receive_from(asio::buffer(recvBuf), p, 0, e)) {
                RecvHandler(e, recvLen);
            }
        }

		// udp 接收回调。kcp input 或 握手
		inline void RecvHandler(asio::error_code const& e, size_t recvLen) {
			if (closed) return;
			if (kcp) {
				// 准备向 kcp 灌数据并收包放入 recvs
				// 前置检查. 如果数据长度不足( kcp header ), 或 conv 对不上就 忽略
				if (recvLen < 24 || conv != *(uint32_t*)(recvBuf)) return;

				// 将数据灌入 kcp. 灌入出错则 Close
				if (int r = ikcp_input(kcp, recvBuf, (long)recvLen)) {
					Close(r, "ikcp_input");
					return;
				}

				// 开始处理收到的数据
				do {
					// 如果数据长度 == buf限长 就自杀( 未处理数据累计太多? )
					if (recv.len == recv.cap) {
						Close(-__LINE__, "recv.len == recv.cap");
						return;
					}

					// 从 kcp 提取数据. 追加填充到 recv 后面区域. 返回填充长度. <= 0 则下次再说
					auto&& len = ikcp_recv(kcp, recv.buf + recv.len, (int)(recv.cap - recv.len));
					if (len <= 0) return;
					recv.len += len;

					// 开始切包放 recvs
					// 取出指针备用
					auto buf = (uint8_t*)recv.buf;
					auto end = (uint8_t*)recv.buf + recv.len;

					// 包头
					uint32_t dataLen = 0;

					// 确保包头长度充足
					while (buf + sizeof(dataLen) <= end) {
						// 读包头 / 数据长
						dataLen = buf[0] | (buf[1] << 8) | (buf[2] << 16) | (buf[3] << 24);

						// 计算包总长( 包头长 + 数据长 )
						auto totalLen = sizeof(dataLen) + dataLen;

						// 如果包不完整 就 跳出
						if (buf + totalLen > end) break;

						// 复制包到 recvs
						recvs.emplace_back(Make<Data>(buf, totalLen));

						// 跳到下一个包的开头
						buf += totalLen;
					}

					// 移除掉已处理的数据( 将后面剩下的数据移动到头部 )
					recv.RemoveFront(buf - (uint8_t*)recv.buf);

				} while (true);
			}
			else {
				// 判断握手信息正确性, 切割出 conv
				if (recvLen == 8 && *(uint32_t*)recvBuf == shakeSerial) {
					conv = *(uint32_t*)(recvBuf + 4);

					// 记录创建毫秒数 for Update
					createMS = NowSteadyEpochMS();

					// 创建并设置 kcp 的一些参数
					kcp = ikcp_create(conv, this);
					(void)ikcp_wndsize(kcp, 1024, 1024);
					(void)ikcp_nodelay(kcp, 1, 10, 2, 1);
					kcp->rx_minrto = 10;
					kcp->stream = 1;
					ikcp_setmtu(kcp, 470);    // 该参数或许能提速, 因为小包优先

					// 给 kcp 绑定 output 功能函数
					ikcp_setoutput(kcp, [](const char* buf, int len, ikcpcb* _, void* user) -> int {
						return (int)((KcpPeer*)user)->socket.send_to(asio::buffer(buf, len), ((KcpPeer*)user)->ep);
						});

					// 先 update 来一发 确保 Flush 生效( 和 kcp 内部实现的一个标志位有关 )
					ikcp_update(kcp, 0);

					// 通过 kcp 发 accept 触发包
					Send("\1\0\0\0\0", 5);
					Flush();

					// 将自己放入 client peer 并停止拨号
					auto&& c = *(Client*)ctx;
					c.peer = As<KcpPeer>(shared_from_this());
					c.Stop();
				}
			}
		}

		inline int Send(char const* const& buf, size_t const& len) {
			if (closed) return -1;
			return ikcp_send(kcp, buf, (int)len);
		}

		// 立刻开始发送
		inline void Flush() {
			if (closed) return;
			ikcp_flush(kcp);
		}

		// 被 Client 每帧调用, 驱动 kcp 逻辑
		inline void Update() {
			if (closed) return;
			ikcp_update(kcp, (IUINT32)(NowSteadyEpochMS() - createMS));
		}

		// 回收 kcp 并清空数据, 撤销超时回调
		bool Close(int const& reason, char const* const& desc) override {
			if (closed) return false;
			if (kcp) {
				ikcp_release(kcp);
				kcp = nullptr;
			}
			recvs.clear();
			ClearTimeout();
			closed = reason;
			closedDesc = desc;
			return true;
		}

		// Close
		inline void Timeout() override {
			Close(-__LINE__, "Timeout");
		}
	};

	inline void Client::Dial(float const& timeoutSeconds) {
		Stop();
		for (auto&& kv : dialAddrs) {
			for (auto&& port : kv.second) {
				dialPeers.emplace_back(Make<KcpPeer>(this, asio::ip::udp::endpoint(kv.first, port), ++shakeSerial));
			}
		}
		// 启动拨号超时检测 timer
		dialTimer.SetTimeout(timeoutSeconds);

		// 启动握手 timer 并立刻发送握手包
		shakeTimer.onTimeout(&shakeTimer);
	}

	inline void Client::Stop() {
		dialPeers.clear();
		shakeTimer.ClearTimeout();
		dialTimer.ClearTimeout();
	}

	inline int Client::FrameUpdate() {
		if (peer) {
			peer->Update();
		}
		return this->BaseType::FrameUpdate();
	}

	inline Client::Client(size_t const& wheelLen, double const& frameRate_)
		: BaseType(wheelLen, frameRate_)
		, shakeTimer(this)
		, dialTimer(this)
	{
		// 设置握手回调: 不停的发送握手包
		shakeTimer.onTimeout = [this](auto t) {
            for (auto&& p : dialPeers) {
                p->socket.send_to(asio::buffer(&p->shakeSerial, sizeof(p->shakeSerial)), p->ep);
            }
			t->SetTimeout(0.2);
		};

		// 设置拨号回调: 到时就 Stop
		dialTimer.onTimeout = [this](auto t) {
			Stop();
		};
	}

	inline bool Client::PeerAlive() {
        return peer && !peer->closed;
	}

    inline int Client::RunOnce(double const& elapsedSeconds) {
        ioc.poll();
        if (Busy()) {
            for (auto &&dp : dialPeers) {
                dp->Recv();
            }
        }
        if (PeerAlive()) {
            peer->Recv();
        }
        secondsPool += elapsedSeconds;
        auto rtv = this->BaseType::RunOnce();
        ioc.poll();
        return rtv;
    }
}
