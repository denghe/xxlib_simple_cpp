#pragma once
#include "xx_epoll.h"
#include "config.h"

namespace EP = xx::Epoll;

// Ԥ����
struct Listener;
using Listener_r = EP::Ref<Listener>;

struct Peer;
using Peer_r = EP::Ref<Peer>;


// ������
struct Service : EP::Context {

	// �ڹ��캯���и��� config ��һ����ʼ��
	Service(size_t const& wheelLen = 1 << 12);

	// ֡�߼���������
	virtual int FrameUpdate() override;

	// ������ǰ���е�һЩ��Ա
	~Service();

	// ������
	Listener_r listener;


};



// �̳�Ĭ�ϼ��������ǹؼ�����
struct Listener : EP::TcpListener {
	// ͸�����캯��
	using EP::TcpListener::TcpListener;

	// �ṩ����Ŀ����ʵ�����ڴ����֧��
	virtual EP::TcpPeer_u OnCreatePeer() override;

	// �����ѽ���, ����
	virtual void OnAccept(EP::TcpPeer_r const& peer_) override;
};




// �̳� Ĭ�� ���Ӹ����հ�����
struct Peer : EP::TcpPeer {

	// �յ����ݵĴ���
	virtual void OnReceive() override;

};







inline EP::TcpPeer_u Listener::OnCreatePeer() {
	// ���� Peer ��ʵ��
	return xx::TryMakeU<Peer>();
}

inline void Listener::OnAccept(EP::TcpPeer_r const& peer_) {
	// ��ԭΪ����������
	auto&& peer = peer_.As<Peer>();

	// ���� 5 ������
	peer->SetTimeoutSeconds(5);
}

inline void Peer::OnReceive() {
	/*
	// todo: ֧�� json
	��ͷ���������ͷ
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
