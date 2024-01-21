#pragma once
#include "./IocpNetwork/SessionSocketCompeletionKey.h"
#include "MsgQueue.h"
class MyWebSocketEndpoint;
class MySession
{
public:
	template<class T> void Send(const T& ref);
	void OnInit(Iocp::SessionSocketCompeletionKey<MySession>& refSession);
	int OnRecv(Iocp::SessionSocketCompeletionKey<MySession> &refSession,const char buf[], int len);
	void OnDestroy();

	MsgQueue msgQueue;
	MyWebSocketEndpoint* ws = nullptr;
private:
	Iocp::SessionSocketCompeletionKey<MySession> *pSession;

};

template<class T>
void Broadcast(const T& msg);

extern std::set<Iocp::SessionSocketCompeletionKey<MySession>*> g_set;
extern std::mutex g_setMutex;