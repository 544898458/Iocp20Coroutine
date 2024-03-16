#include "pch.h"
#pragma comment(lib, "Ws2_32.lib")
#pragma comment(lib, "glog.lib")
#pragma comment(lib, "Mswsock.lib")

#include "../IocpNetwork/ServerTemplate.h"
#include "../IocpNetwork/ListenSocketCompeletionKeyTemplate.h"
#include "../IocpNetwork/SessionSocketCompeletionKeyTemplate.h"
class WorldServer;
class WorldSession
{
public:
	void OnInit(Iocp::SessionSocketCompeletionKey<WorldSession>& session, WorldServer&)
	{
		
	}
	int OnRecv(Iocp::SessionSocketCompeletionKey<WorldSession>& refSession, const char buf[], int len) 
	{
		return len;
	}
	void OnDestroy() 
	{

	}
};
class WorldServer 
{
public:
	void OnAdd(Iocp::SessionSocketCompeletionKey<WorldSession>& session)
	{

	}
	
};


template Iocp::Server<WorldServer>;
template bool Iocp::Server<WorldServer>::Init<WorldSession>();
template void Iocp::ListenSocketCompeletionKey::StartCoRoutine<WorldSession, WorldServer >(HANDLE hIocp, SOCKET socketListen, WorldServer&);
template Iocp::SessionSocketCompeletionKey<WorldSession>;

int main()
{
	Iocp::Server<WorldServer> accept;
	accept.WsaStartup();
	accept.Init<WorldSession>();
}
