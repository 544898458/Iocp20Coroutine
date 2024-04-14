#include "WorldServer.h"
#include "../IocpNetwork/ServerTemplate.h"
#include "WorldSession.h"
#include "../IocpNetwork/ListenSocketCompeletionKeyTemplate.h"
template Iocp::Server<WorldServer>;
template bool Iocp::Server<WorldServer>::Init<WorldSession>(const uint16_t);
template void Iocp::ListenSocketCompeletionKey::StartCoRoutine<WorldSession, WorldServer >(HANDLE hIocp, SOCKET socketListen, WorldServer&);
