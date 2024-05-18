#include "WorldSession.h"
#include "../IocpNetwork/SessionSocketCompeletionKeyTemplate.h"
#include "../IocpNetwork/SessionsTemplate.h"
#include "WorldServer.h"

template Iocp::SessionSocketCompeletionKey<WorldSession>;

int WorldSession::OnRecv(CompeletionKeySession& refSession, const char buf[], int len)
{
	return 0;
}

void WorldSession::OnInit(CompeletionKeySession& refSession, WorldServer& refServer)
{
	refServer.m_Sessions.AddSession(&refSession, [this, &refSession, &refServer]()
		{
			LOG(INFO) << "WorldSvr已连上";
			//m_pServer = &server;
			m_pSession = &refSession;

			//m_entity.Init(5, m_pServer->m_space, TraceEnemy, this);
			//m_pServer->m_space.setEntity.insert(&m_entity);
		});
}

void WorldSession::OnDestroy()
{
	//m_entity.OnDestroy();
	//m_pServer->m_Sessions.DeleteSession(this->m_pWsSession->m_pSession, [this]()
	//	{
	//		m_pServer->m_space.setEntity.erase(&m_entity);
	//	});

	//m_pServer = nullptr;//不用加锁
}