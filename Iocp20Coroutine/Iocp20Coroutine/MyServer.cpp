#include "StdAfx.h"
#include "MyServer.h"
#include "../IocpNetwork/Server.h"
#include "../IocpNetwork/ServerTemplate.h"
#include "MySession.h"
#include "../IocpNetwork/ListenSocketCompletionKeyTemplate.h"
#include "../IocpNetwork/SessionsTemplate.h"

template Iocp::Server<MyServer>;
template bool Iocp::Server<MyServer>::Init<WebSocketSession<MySession> >(const uint16_t);
template void Iocp::ListenSocketCompletionKey::StartCoRoutine<WebSocketSession<MySession>, MyServer >(HANDLE hIocp, SOCKET socketListen, MyServer&);

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
		LOG(INFO) << "ÒÑÉ¾³ý¶ÔÏó,GetCurrentThreadId=" << GetCurrentThreadId();

	}
	m_space.Update();
	*/
}


template void Sessions<MySession>::Broadcast<MsgAddRoleRet >(const MsgAddRoleRet&);
template void Sessions<MySession>::Broadcast<MsgNotifyPos>(const MsgNotifyPos&);
template void Sessions<MySession>::Broadcast<MsgChangeSkeleAnim>(const MsgChangeSkeleAnim&);