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
	/// ���������ӹ㲥��Ϣ
	/// </summary>
	/// <typeparam name="T"></typeparam>
	/// <param name="msg"></param>
	template<class T>
	void Broadcast(const T& msg);
	Space g_space;
//private:
	/// <summary>
	/// ���л����
	/// </summary>
	std::set<Session*> m_setSession;
	/// <summary>
	/// ���߳�ȫ�ֲ���g_setSession
	/// </summary>
	std::recursive_mutex m_setSessionMutex;
};

