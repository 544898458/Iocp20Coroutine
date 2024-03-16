#pragma once
#include "../IocpNetwork/Server.h"
#include "../IocpNetwork/SessionSocketCompeletionKey.h"
#include "MySession.h"
#include "Space.h"

class MyServer
{
public:
	using Session = Iocp::SessionSocketCompeletionKey<WebSocketSession< MySession> >;
	void OnAdd(Session&);
	void OnDel();
	void Update();

	/// <summary>
	/// 向所有连接广播消息
	/// </summary>
	/// <typeparam name="T"></typeparam>
	/// <param name="msg"></param>
	template<class T>
	void Broadcast(const T& msg);
	Space g_space;
//private:
	/// <summary>
	/// 所有活动连接
	/// </summary>
	std::set<Session*> m_setSession;
	/// <summary>
	/// 多线程全局操作g_setSession
	/// </summary>
	std::recursive_mutex m_setSessionMutex;
};

