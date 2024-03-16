#pragma once
#include "./IocpNetwork/Server.h"
#include "IocpNetwork/SessionSocketCompeletionKey.h"
#include "MySession.h"

class MyServer
{
public:
	using Session = Iocp::SessionSocketCompeletionKey<WebSocketSession< MySession> >;
	void OnAdd(Session&);
	void OnDel();

	/// <summary>
	/// ���л����
	/// </summary>
	std::set<Session*> g_setSession;
	/// <summary>
	/// ���߳�ȫ�ֲ���g_setSession
	/// </summary>
	std::mutex g_setSessionMutex;
};

