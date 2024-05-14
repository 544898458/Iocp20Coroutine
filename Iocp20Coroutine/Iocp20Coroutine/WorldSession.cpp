#include "WorldSession.h"
#include "../IocpNetwork/SessionSocketCompeletionKeyTemplate.h"
#include "WorldServer.h"

template Iocp::SessionSocketCompeletionKey<WorldSession>;

int WorldSession::OnRecv(Iocp::SessionSocketCompeletionKey<WorldSession>& refSession, const char buf[], int len)
{
	return 0;
}

void WorldSession::OnInit(CompeletionKeySession& refSession, WorldServer& refServer)
{
	refServer.m_Sessions.AddSession(&refSession, [this, &refSession, &refServer]()
		{
			//m_pServer = &server;
			//m_pWsSession = pWsSession;

			//m_entity.Init(5, m_pServer->m_space, TraceEnemy, this);
			//m_pServer->m_space.setEntity.insert(&m_entity);
		});
}