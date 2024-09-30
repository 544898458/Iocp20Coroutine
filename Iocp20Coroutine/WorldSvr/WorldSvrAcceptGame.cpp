#include "pch.h"
#include "WorldSvrAcceptGame.h"
#include "../IocpNetwork/ServerTemplate.h"
#include "WorldSession.h"
#include "../IocpNetwork/ListenSocketCompletionKeyTemplate.h"
//#include "../IocpNetwork/WebSocketSession.h"

template Iocp::Server<WorldSvrAcceptGame>;
template bool Iocp::Server<WorldSvrAcceptGame>::Init<WorldSessionFromGame>(const uint16_t);
template void Iocp::ListenSocketCompletionKey::StartCoRoutine<WorldSessionFromGame , WorldSvrAcceptGame >(HANDLE hIocp, SOCKET socketListen, WorldSvrAcceptGame&);

void WorldSvrAcceptGame::OnAdd(CompeletionKeySession&)
{
}
