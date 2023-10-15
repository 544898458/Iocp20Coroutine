#pragma once
#include "SessionSocketCompeletionKey.h"
class MySession
{
public:
	int OnRecv(Iocp::SessionSocketCompeletionKey<MySession> &refSession,const char buf[], int len);
};

