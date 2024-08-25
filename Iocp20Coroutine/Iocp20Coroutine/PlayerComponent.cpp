#include "StdAfx.h"
#include "PlayerComponent.h"
#include "Entity.h"
#include "GameSvrSession.h"
#include "PlayerGateSession.h"

void Entity::AddComponentPlayer(PlayerGateSession &refSession)
{
	m_spPlayer = std::make_shared<PlayerComponent, PlayerGateSession&>(refSession);
}
