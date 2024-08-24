#include "GateServer.h"
#include "../IocpNetwork/ServerTemplate.h"
#include "../IocpNetwork/ListenSocketCompletionKeyTemplate.h"

template Iocp::Server<GateServer>;
template bool Iocp::Server<GateServer>::Init<GateSession>(const uint16_t);
template void Iocp::ListenSocketCompletionKey::StartCoRoutine<GateSession, GateServer >(HANDLE hIocp, SOCKET socketListen, GateServer&);

void GateServer::OnAdd(CompeletionKeySession&)
{
}
