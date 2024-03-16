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
	/// 所有活动连接
	/// </summary>
	std::set<Session*> g_setSession;
	/// <summary>
	/// 多线程全局操作g_setSession
	/// </summary>
	std::mutex g_setSessionMutex;
};

