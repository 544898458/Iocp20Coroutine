#pragma once
#include "../IocpNetwork/Server.h"
#include "../IocpNetwork/SessionSocketCompeletionKey.h"
#include "MySession.h"
#include "Space.h"
#include "../IocpNetwork/Sessions.h"

class MyServer
{
public:
	MyServer(const MyServer&) = delete;
	MyServer(MyServer&&) = delete;
	MyServer() {}
	using Session = Iocp::SessionSocketCompeletionKey<WebSocketSession< MySession> >;
	void OnAdd(Session&);
	void OnDel();
	void Update();

	Space m_space;
	Sessions< WebSocketSession< MySession> > m_Sessions;

};