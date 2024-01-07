#pragma once
#include "SessionSocketCompeletionKey.h"
class MyWebSocketEndpoint;
class MySession
{
public:
	void OnInit(Iocp::SessionSocketCompeletionKey<MySession>& refSession);
	int OnRecv(Iocp::SessionSocketCompeletionKey<MySession> &refSession,const char buf[], int len);
	void OnDestroy();
private:
	MyWebSocketEndpoint* ws = nullptr;
};

