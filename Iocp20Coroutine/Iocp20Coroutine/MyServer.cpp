#include "MyServer.h"
#include "./IocpNetwork/Server.h"
#include "MyServer.h"
#include "IocpNetwork/ServerTemplate.h"
#include "MySession.h"
#include "IocpNetwork/ListenSocketCompeletionKeyTemplate.h"

template Iocp::Server<MyServer>;

template bool Iocp::Server<MyServer>::Init<WebSocketSession<MySession> >();
template void Iocp::ListenSocketCompeletionKey::StartCoRoutine<WebSocketSession<MySession>, MyServer >(HANDLE hIocp, SOCKET socketListen, MyServer&);

void MyServer::OnAdd(Iocp::SessionSocketCompeletionKey<WebSocketSession<MySession>>&)
{
}
