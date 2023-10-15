#pragma once
#include "SessionSocketCompeletionKey.h"
class MySession
{
public:
	int OnRecv(SessionSocketCompeletionKey<MySession> &refSession,const char buf[], int len);
};

