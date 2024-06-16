#include "pch.h"
#include "WorldServer.h"
#include "../IocpNetwork/ServerTemplate.h"
#include "WorldSession.h"
#include "../IocpNetwork/ListenSocketCompeletionKeyTemplate.h"
#include "../IocpNetwork/WebSocketSession.h"

template Iocp::Server<WorldServer>;
template bool Iocp::Server<WorldServer>::Init< WorldSession >(const uint16_t);
template void Iocp::ListenSocketCompeletionKey::StartCoRoutine<WorldSession , WorldServer >(HANDLE hIocp, SOCKET socketListen, WorldServer&);

void WorldServer::OnAdd(CompeletionKeySession&)
{
}
