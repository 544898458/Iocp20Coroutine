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
void GameSvr::Update()
{
	m_Sessions.Update([this]()
		{
			//Space::StaticUpdate();
		});
}

void GameSvr::OnAppExit()
{
}


template void Sessions<GameSvrSession>::Broadcast<MsgAddRoleRet >(const MsgAddRoleRet&);
template void Sessions<GameSvrSession>::Broadcast<MsgNotifyPos>(const MsgNotifyPos&);
template void Sessions<GameSvrSession>::Broadcast<MsgChangeSkeleAnim>(const MsgChangeSkeleAnim&);