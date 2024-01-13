#pragma once
#include "SessionSocketCompeletionKey.h"
#include "MsgQueue.h"
class MyWebSocketEndpoint;
class MySession
{
public:
	void OnInit(Iocp::SessionSocketCompeletionKey<MySession>& refSession);
	int OnRecv(Iocp::SessionSocketCompeletionKey<MySession> &refSession,const char buf[], int len);
	void OnDestroy();

	MsgQueue msgQueue;
private:
	MyWebSocketEndpoint* ws = nullptr;

};

extern std::set<MySession*> g_set;