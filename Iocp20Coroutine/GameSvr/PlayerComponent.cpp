#include "pch.h"
#include "PlayerComponent.h"
#include "Entity.h"
#include "GameSvrSession.h"
#include "PlayerGateSession_Game.h"
#include "../IocpNetwork/StrConv.h"

void PlayerComponent::AddComponent(Entity& refEntity, PlayerGateSession_Game& refSession)
{
	refEntity.m_spPlayer = std::make_shared<PlayerComponent, PlayerGateSession_Game&>(refSession);
}

void PlayerComponent::Say(const std::string& str, const SayChannel channel)
{
	m_refSession.Say(str, channel);
}
