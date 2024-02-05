#pragma once
#include "./IocpNetwork/SessionSocketCompeletionKey.h"
#include "MsgQueue.h"
#include "Entity.h"
class MyWebSocketEndpoint;
class MySession
{
public:
	MySession();
	template<class T> void Send(const T& ref);
	void OnInit(Iocp::SessionSocketCompeletionKey<MySession>& refSession);
	int OnRecv(Iocp::SessionSocketCompeletionKey<MySession> &refSession,const char buf[], int len);
	void OnDestroy();

	MsgQueue msgQueue;
	MyWebSocketEndpoint* ws = nullptr;
	Entity entity;
private:
	Iocp::SessionSocketCompeletionKey<MySession> *pSession;
	

};

template<class T>
void Broadcast(const T& msg);

extern std::set<Iocp::SessionSocketCompeletionKey<MySession>*> g_set;
extern std::mutex g_setMutex;