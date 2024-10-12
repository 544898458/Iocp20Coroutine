#include "pch.h"
#include "PlayerComponent.h"
#include "Entity.h"
#include "GameSvrSession.h"
#include "PlayerGateSession_Game.h"
#include "../IocpNetwork/StrConv.h"

void Entity::AddComponentPlayer(PlayerGateSession_Game &refSession)
{
	m_spPlayer = std::make_shared<PlayerComponent, PlayerGateSession_Game&>(refSession);
}

void PlayerComponent::Say(const std::string& str)
{
	m_refSession.Say(str);
}
