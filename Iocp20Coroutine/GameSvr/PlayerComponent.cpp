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

void PlayerComponent::播放声音(Entity& refEntity, const std::string& refStr声音, const std::string& str文本)
{
	if (refEntity.m_spPlayer)
		refEntity.m_spPlayer->m_refSession.播放声音(refStr声音, str文本);
}

void PlayerComponent::Send资源(Entity& refEntity)
{
	if (refEntity.m_spPlayer)
		refEntity.m_spPlayer->m_refSession.Send资源();
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
