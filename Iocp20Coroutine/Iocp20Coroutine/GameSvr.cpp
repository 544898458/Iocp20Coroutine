#include "pch.h"
#include "GameSvr.h"
//#include "../IocpNetwork/Server.h"
#include "../IocpNetwork/ServerTemplate.h"
#include "GameSvrSession.h"
#include "../IocpNetwork/ListenSocketCompletionKeyTemplate.h"
//#include "../IocpNetwork/SessionsTemplate.h"

template class Iocp::Server<GameSvr>;
template bool Iocp::Server<GameSvr>::Init<GameSvrSession>(const uint16_t);
template void Iocp::ListenSocketCompletionKey::StartCoRoutine<GameSvrSession, GameSvr >(HANDLE hIocp, SOCKET socketListen, GameSvr&);

GameSvr::GameSvr()
{
}

void GameSvr::OnAdd(Session&)
{
}

void GameSvr::OnDel()
{
}
extern Space g_Space无限刷怪;
void GameSvr::Update()
{
	m_Sessions.Update([this]()
		{
			g_Space无限刷怪.Update();
		});
	
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


template void Sessions<GameSvrSession>::Broadcast<MsgAddRoleRet >(const MsgAddRoleRet&);
template void Sessions<GameSvrSession>::Broadcast<MsgNotifyPos>(const MsgNotifyPos&);
template void Sessions<GameSvrSession>::Broadcast<MsgChangeSkeleAnim>(const MsgChangeSkeleAnim&);