#include <glog/logging.h>

#include "IocpNetwork/ServerTemplate.h"
#include "IocpNetwork/ListenSocketCompeletionKey.cpp"
#include "IocpNetwork/SessionSocketCompeletionKey.cpp"
#include "MySession.h"

//#include <iostream>
#include <cassert>

#include <codecvt>
#include "MsgQueue.h"
#include "Space.h"
#include "CoTimer.h"

//template<MySession>
//std::set<Iocp::SessionSocketCompeletionKey<MySession>*> g_setSession;
//template<MySession> std::mutex g_setSessionMutex;


template bool Iocp::Server::Init<WebSocketSession<MySession> >();
template class Iocp::SessionSocketCompeletionKey<WebSocketSession<MySession> >;


template<class T>
void MySession::Send(const T& ref)
{
	m_webSocketEndpoint->Send(ref);
}


CoTask<int> TraceEnemy(Entity* pEntity, float& x, float& z, std::function<void()>& funCancel)
{
	KeepCancel kc(funCancel);
	bool stop = false;
	funCancel = [&stop]() {stop = true; };
	while (true)
	{
		if (co_await CoTimer::WaitNextUpdate(funCancel))
		{
			LOG(INFO) << "调用者手动取消了协程TraceEnemy";
			co_return 0;
		}
		if (stop)
		{

			LOG(INFO) << "TraceEnemy协程正常退出";
			co_return 0;
		}
		x -= 0.01;

		Broadcast<MsgNotifyPos,WebSocketSession<MySession>>(MsgNotifyPos(pEntity, x, z));
	}
}

MySession::MySession() : m_entity(5, g_space, TraceEnemy), m_msgQueue(this)
{
}
template<class T_Session>
void WebSocketSession<T_Session>::OnInit(Iocp::SessionSocketCompeletionKey<WebSocketSession<T_Session>>& refSession)
{
	std::lock_guard lock(g_setSessionMutex<WebSocketSession<T_Session>>);
	m_webSocketEndpoint.reset(new MyWebSocketEndpoint<T_Session>(net_write_cb, &refSession));

	m_pSession = &refSession;
	g_space.setEntity.insert(&m_entity);
}
template<class T_Session>
int WebSocketSession< T_Session>::OnRecv(Iocp::SessionSocketCompeletionKey<WebSocketSession<T_Session>>& refSession, const char buf[], int len)
{
	m_webSocketEndpoint->from_wire(buf, len);
	return len;
}
template<class T_Session>
void WebSocketSession< T_Session>::OnDestroy()
{
	std::lock_guard lock(g_setSessionMutex<WebSocketSession>);
	g_setSession<WebSocketSession>.erase(this->m_pSession);
	g_space.setEntity.erase(&m_entity);
	LOG(INFO) << "删除Session，剩余" << g_setSession<WebSocketSession>.size();

}

template void MySession::Send(const MsgLoginRet&);
template void MySession::Send(const MsgNotifyPos&);

template<class T,class T_Session>
void Broadcast(const T& msg)
{
	std::lock_guard lock(g_setSessionMutex<T_Session>);
	for (auto p : g_setSession<T_Session>)
	{
		p->Session.Send(msg);
	}
}

template void Broadcast<MsgLoginRet,WebSocketSession<MySession>>(const MsgLoginRet&);
template void Broadcast<MsgNotifyPos, WebSocketSession<MySession>>(const MsgNotifyPos&);
template void Broadcast<MsgChangeSkeleAnim, WebSocketSession<MySession>>(const MsgChangeSkeleAnim&);