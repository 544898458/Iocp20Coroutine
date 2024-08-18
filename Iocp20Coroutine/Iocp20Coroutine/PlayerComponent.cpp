#include "StdAfx.h"
#include "PlayerComponent.h"
#include "Entity.h"

void Entity::AddComponentPlayer(GameSvrSession* pSession)
{
	CHECK_NOTNULL(pSession);
	if (nullptr == pSession)
		return;

	m_spPlayer = std::make_shared<PlayerComponent>();
	m_spPlayer->m_pSession = pSession;
}
