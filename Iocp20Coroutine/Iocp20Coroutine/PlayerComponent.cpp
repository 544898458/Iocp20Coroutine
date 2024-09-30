#include "pch.h"
#include "PlayerComponent.h"
#include "Entity.h"
#include "GameSvrSession.h"
#include "PlayerGateSession.h"

void Entity::AddComponentPlayer(PlayerGateSession_Game &refSession)
{
	m_spPlayer = std::make_shared<PlayerComponent, PlayerGateSession_Game&>(refSession);
}
