#include "pch.h"
#include "PlayerComponent.h"
#include "Entity.h"
#include "GameSvrSession.h"
#include "PlayerGateSession_Game.h"
#include "../IocpNetwork/StrConv.h"
#include "PlayerNickNameComponent.h"

void PlayerComponent::Say(Entity& refEntity, const std::string& str, const SayChannel channel)
{
	if (refEntity.m_spPlayer)
		refEntity.m_spPlayer->Say(str, channel);
}

void PlayerComponent::��������(Entity& refEntity, const std::string& refStr����, const std::string& str�ı�)
{
	if (refEntity.m_spPlayer)
		refEntity.m_spPlayer->m_refSession.��������(refStr����, str�ı�);
}

void PlayerComponent::Send��Դ(Entity& refEntity)
{
	if (refEntity.m_spPlayer)
		refEntity.m_spPlayer->m_refSession.Send��Դ();
}

void PlayerComponent::AddComponent(Entity& refEntity, PlayerGateSession_Game& refSession)
{
	refEntity.m_spPlayer = std::make_shared<PlayerComponent, PlayerGateSession_Game&>(refSession);
	if(!refEntity.m_spPlayerNickName)
		refEntity.m_spPlayerNickName = std::make_shared<PlayerNickNameComponent, const std::string&>(refSession.NickName());
}

void PlayerComponent::Say(const std::string& str, const SayChannel channel)
{
	m_refSession.Say(str, channel);
}
