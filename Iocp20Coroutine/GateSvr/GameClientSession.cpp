#include "GameClientSession.h"
#include "../IocpNetwork/SessionSocketCompletionKeyTemplate.h"

//template void Iocp::ListenSocketCompletionKey::StartCoRoutine<WorldClientSession, WorldClient >(HANDLE hIocp, SOCKET socketListen, WorldClient&);
template Iocp::SessionSocketCompletionKey<GameClientSession>;
//std::function<void(MsgSay const&)> WorldClient::m_funBroadcast;

int GameClientSession::OnRecv(Iocp::SessionSocketCompletionKey<GameClientSession>& refSession, const void* buf, int len)
{
	return 0;
}
