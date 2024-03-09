#pragma once
#include "./IocpNetwork/Server.h"
#include "IocpNetwork/SessionSocketCompeletionKey.h"
#include "MySession.h"

class MyServer
{
public:
	void OnAdd(Iocp::SessionSocketCompeletionKey<WebSocketSession< MySession> >&);
	void OnDel();
};

