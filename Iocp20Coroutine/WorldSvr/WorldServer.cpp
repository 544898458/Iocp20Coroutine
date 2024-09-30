#include "pch.h"
#include "WorldServer.h"
#include "../IocpNetwork/ServerTemplate.h"
#include "WorldSession.h"
#include "../IocpNetwork/ListenSocketCompletionKeyTemplate.h"
//#include "../IocpNetwork/WebSocketSession.h"

template Iocp::Server<WorldServerAcceptGame>;
template bool Iocp::Server<WorldServerAcceptGame>::Init<WorldSessionFromGame>(const uint16_t);
template void Iocp::ListenSocketCompletionKey::StartCoRoutine<WorldSessionFromGame , WorldServerAcceptGame >(HANDLE hIocp, SOCKET socketListen, WorldServerAcceptGame&);

void WorldServerAcceptGame::OnAdd(CompeletionKeySession&)
{
}
