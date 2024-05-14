#include "MyServer.h"
#include "../IocpNetwork/Server.h"
#include "../IocpNetwork/ServerTemplate.h"
#include "MySession.h"
#include "../IocpNetwork/ListenSocketCompeletionKeyTemplate.h"
#include "../IocpNetwork/SessionsTemplate.h"

template Iocp::Server<MyServer>;
template bool Iocp::Server<MyServer>::Init<WebSocketSession<MySession> >(const uint16_t);
template void Iocp::ListenSocketCompeletionKey::StartCoRoutine<WebSocketSession<MySession>, MyServer >(HANDLE hIocp, SOCKET socketListen, MyServer&);

template<class T_Session>
template<typename T_Function>
void Sessions<T_Session>::Update(T_Function const& functionLockUpdate)
{
	std::lock_guard lock(m_setSessionMutex);
	std::set<Session*> setDelete;
	for (Session* p : m_setSession)
	{
		p->Session.m_Session.m_msgQueue.Process();
		if (p->Finished())
			setDelete.insert(p);
	}
	for (auto p : setDelete)
	{
		p->Session.OnDestroy();
		delete p;
		LOG(INFO) << "已删除对象,GetCurrentThreadId=" << GetCurrentThreadId();

	}
	functionLockUpdate();
}


void MyServer::OnAdd(Session&)
{
}

void MyServer::OnDel()
{
}
void MyServer::Update()
{
	m_Sessions.Update([this](){m_space.Update();});
	/*
	std::lock_guard lock(m_Sessions.m_setSessionMutex);
	std::set<MyServer::Session*> setDelete;
	for (auto p : m_Sessions.m_setSession)
	{
		p->Session.m_Session.m_msgQueue.Process();
		if (p->Finished())
			setDelete.insert(p);
	}
	for (auto p : setDelete)
	{
		p->Session.OnDestroy();
		delete p;
		LOG(INFO) << "已删除对象,GetCurrentThreadId=" << GetCurrentThreadId();

	}
	m_space.Update();
	*/
}


template void Sessions<MySession>::Broadcast<MsgLoginRet >(const MsgLoginRet&);
template void Sessions<MySession>::Broadcast<MsgNotifyPos>(const MsgNotifyPos&);
template void Sessions<MySession>::Broadcast<MsgChangeSkeleAnim>(const MsgChangeSkeleAnim&);