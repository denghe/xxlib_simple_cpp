#pragma once
#include "xx_epoll.h"

namespace EP = xx::Epoll;

// Ԥ����
struct Dialer;
using Dialer_r = EP::Ref<Dialer>;

struct Peer;
using Peer_r = EP::Ref<Peer>;


// ������
struct Client : EP::Context {
	// �ڹ��캯���и��� config ��һ����ʼ��
	Client(size_t const& wheelLen = 1 << 12);

	// ֡�߼���������
	virtual int FrameUpdate() override;

	// ������ǰ���е�һЩ��Ա
	~Client();

	// ������
	Dialer_r dialer;

	// ���������ӳɹ��� peer ���ڴ��Ա�ʹ��
	Peer_r peer;
};


// �̳�Ĭ�ϼ��������ǹؼ�����
struct Dialer : EP::TcpDialer {
	// ͸�����캯��
	using EP::TcpDialer::TcpDialer;

	// �ṩ����Ŀ����ʵ�����ڴ����֧��
	virtual EP::TcpPeer_u OnCreatePeer() override;

	// �����ѽ���, ����
	virtual void OnConnect(EP::TcpPeer_r const& peer_) override;
};


// �̳� Ĭ�� ���Ӹ����հ�����
struct Peer : EP::TcpPeer {
	// ��������¼�
	void HandleReceive(char* buf, size_t len);

	// �յ�����
	virtual void OnReceive() override;

	// �����¼�
	virtual void OnDisconnect(int const& reason) override;

	// ������ǰ�渽���� ���� ������. ���� ��0 ��ʾ��״��( ����һ���Ƕ��� )
	int SendPackage(char const* const& buf, size_t const& len);
};


// ��ͷ. ���ṹ�򵥵Ķ�Ϊ  header(len) + data
struct Header {
	uint16_t len;
};


inline EP::TcpPeer_u Dialer::OnCreatePeer() {
	// ���� Peer ��ʵ��
	return xx::TryMakeU<Peer>();
}

inline void Dialer::OnConnect(EP::TcpPeer_r const& peer_) {
	// û����
	if (!peer_) return;

	// �õ� client ������
	auto c = (Client*)ep;

	// �� peer ��ŵ� c ����
	c->peer = peer_.As<Peer>();

	std::cout << "OnConnect" << std::endl;
}

inline void Peer::OnDisconnect(int const& reason) {
	std::cout << "OnDisconnect. reason = " << reason << std::endl;
}

inline int Peer::SendPackage(char const* const& buf, size_t const& len) {
	// ��󳤶ȼ��
	if (len > std::numeric_limits<decltype(Header::len)>::max()) return -9999;

	// �����ͷ
	Header h;
	h.len = (decltype(Header::len))len;

	// ���������������ƴ��
	xx::Data d(sizeof(h) + len);
	d.WriteBuf((char*)&h, sizeof(h));
	d.WriteBuf(buf, len);

	// ����
	return Send(std::move(d));
}


inline void Peer::OnReceive() {
	// �����жϱ���
	EP::Ref<Item> alive(this);

	// ����ƫ��
	size_t offset = 0;

	// ��ͷ����
	Header h;

	// ȷ����ͷ���ȳ���
	while (offset + sizeof(h) <= recv.len) {

		// ��������ͷ����
		memcpy(&h, recv.buf + offset, sizeof(h));

		// ����δ������ �� ����
		if (offset + sizeof(h) + h.len > recv.len) break;

		// ƫ���� ָ�� ������
		offset += sizeof(h);

		// ���ô�����
		HandleReceive(recv.buf + offset, h.len);

		// �����ǰ��ʵ������ɱ���˳�
		if (!alive) return;

		// ������һ�����Ŀ�ͷ
		offset += h.len;
	}

	// �Ƴ����Ѵ��������( ������ʣ�µ������ƶ���ͷ�� )
	recv.RemoveFront(offset);
}

inline void Peer::HandleReceive(char* buf, size_t len) {
	// ����������յ�������
	std::cout << "recv message from server: " << std::string_view(buf, len) << std::endl;

	// todo: ���ݴ���
}


inline Client::Client(size_t const& wheelLen) : EP::Context(wheelLen) {

	// ��ʼ��������
	this->dialer = CreateTcpDialer<Dialer>();
	if (!this->dialer) {
		throw - 1;
	}

	// ���Ĭ�ϲ��ŵ�ַ
	dialer->AddAddress("127.0.0.1", 10000);

	// ע�ύ��ָ��
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
	// �Զ����� & �����߼�
	// ���δ�������Ӳ��Ҳ�����û�����ڲ���, �Ϳ�ʼ����
	if (!peer && !dialer->Busy()) {
		// ��ʱʱ�� 2 ��
		dialer->DialSeconds(2);
	}

	//std::cout << ".";
	//std::cout.flush();
	return 0;
}

inline Client::~Client() {
	// todo
}
