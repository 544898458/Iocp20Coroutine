#pragma once
#include "../IocpNetwork/Server.h"
#include "../IocpNetwork/SessionSocketCompletionKey.h"
#include "MySession.h"
#include "Space.h"
#include "../IocpNetwork/Sessions.h"

class MyServer
{
public:
	MyServer(const MyServer&) = delete;
	MyServer(MyServer&&) = delete;
	MyServer():m_space(this) {}
	using Session = Iocp::SessionSocketCompletionKey<WebSocketSession< MySession> >;
	void OnAdd(Session&);
	void OnDel();
	void Update();

	Space m_space;
	Sessions< WebSocketSession< MySession> > m_Sessions;

};