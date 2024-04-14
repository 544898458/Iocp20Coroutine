#include "MyServer.h"
#include "../IocpNetwork/Server.h"
#include "../IocpNetwork/ServerTemplate.h"
#include "MySession.h"
#include "../IocpNetwork/ListenSocketCompeletionKeyTemplate.h"

template Iocp::Server<MyServer>;
template bool Iocp::Server<MyServer>::Init<WebSocketSession<MySession> >(const uint16_t);
template void Iocp::ListenSocketCompeletionKey::StartCoRoutine<WebSocketSession<MySession>, MyServer >(HANDLE hIocp, SOCKET socketListen, MyServer&);

void MyServer::OnAdd(Session&)
{
}

void MyServer::OnDel()
{
}
void MyServer::Update()
{
	std::lock_guard lock(m_setSessionMutex);
	std::set<MyServer::Session*> setDelete;
	for (auto p : m_setSession)
	{
		p->Session.m_Session.m_msgQueue.Process();
		if (p->Finished())
			setDelete.insert(p);
	}
	for (auto p : setDelete)
	{
		p->Session.OnDestroy();
		delete p;
		LOG(INFO) << "ÒÑÉ¾³ý¶ÔÏó,GetCurrentThreadId=" << GetCurrentThreadId();

	}
	g_space.Update();
}
template<class T>
void MyServer::Broadcast(const T& msg)
{
	std::lock_guard lock(m_setSessionMutex);
	for (auto p : m_setSession)
	{
		p->Session.m_Session.Send(msg);
	}
}

template void MyServer::Broadcast<MsgLoginRet >(const MsgLoginRet&);
template void MyServer::Broadcast<MsgNotifyPos>(const MsgNotifyPos&);
template void MyServer::Broadcast<MsgChangeSkeleAnim>(const MsgChangeSkeleAnim&);