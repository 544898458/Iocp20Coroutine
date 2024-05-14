#pragma once
#include"../IocpNetwork/SessionSocketCompeletionKey.h"
class WorldServer;
class WorldSession
{
public:
	int OnRecv(Iocp::SessionSocketCompeletionKey<WorldSession>& refSession, const char buf[], int len);
	void OnDestroy();
	using CompeletionKeySession = Iocp::SessionSocketCompeletionKey<WorldSession>;
	void OnInit(CompeletionKeySession& refSession, WorldServer&);
	template<class T>
	void Send(const T& ref);
};

