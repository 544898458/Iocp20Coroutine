#include "pch.h"
#include "WorldSvrAcceptGate.h"
#include "../IocpNetwork/ServerTemplate.h"
#include "WorldSessionFromGame.h"
#include "../IocpNetwork/ListenSocketCompletionKeyTemplate.h"
//#include "../IocpNetwork/WebSocketSession.h"

template Iocp::Server<WorldSvrAcceptGate>;
template bool Iocp::Server<WorldSvrAcceptGate>::Init<WorldSessionFromGate>(const uint16_t);
template void Iocp::ListenSocketCompletionKey::StartCoRoutine<WorldSessionFromGate, WorldSvrAcceptGate >(HANDLE hIocp, SOCKET socketListen, WorldSvrAcceptGate&);

void WorldSvrAcceptGate::OnAdd(CompeletionKeySession&)
{
}