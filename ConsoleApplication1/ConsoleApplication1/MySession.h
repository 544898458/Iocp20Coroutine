#pragma once
#include "SessionSocketCompeletionKey.h"
class MyWebSocketEndpoint;
class MySession
{
public:
	void Init(Iocp::SessionSocketCompeletionKey<MySession>& refSession);
	int OnRecv(Iocp::SessionSocketCompeletionKey<MySession> &refSession,const char buf[], int len);
private:
	MyWebSocketEndpoint* ws = nullptr;
};

