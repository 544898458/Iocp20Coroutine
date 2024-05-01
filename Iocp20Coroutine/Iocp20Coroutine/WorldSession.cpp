#include "WorldSession.h"
#include "../IocpNetwork/SessionSocketCompeletionKeyTemplate.h"

template Iocp::SessionSocketCompeletionKey<WorldSession>;

int WorldSession::OnRecv(Iocp::SessionSocketCompeletionKey<WorldSession>& refSession, const char buf[], int len)
{
	return 0;
}

void WorldSession::OnInit(CompeletionKeySession& refSession, WorldServer&)
{
}