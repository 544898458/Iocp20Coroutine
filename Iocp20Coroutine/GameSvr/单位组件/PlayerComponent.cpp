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

void PlayerComponent::Say系统(Entity& refEntity, const std::string& str)
{
	if (refEntity.m_spPlayer)
		refEntity.m_spPlayer->m_refSession.Say系统(str);
}

void PlayerComponent::剧情对话(Entity& refEntity, const std::string& str头像左, const std::string& str头像右, const std::string& str)
{
	if (refEntity.m_spPlayer)
		refEntity.m_spPlayer->m_refSession.剧情对话(str头像左, str头像右, str);
}

void PlayerComponent::剧情对话已看完(Entity& refEntity)
{
	if (refEntity.m_spPlayer)
		refEntity.m_spPlayer->m_refSession.剧情对话已看完();
}

void PlayerComponent::播放声音(Entity& refEntity, const std::string& refStr声音, const std::string& str文本)
{
	if (refEntity.m_spPlayer)
		refEntity.m_spPlayer->m_refSession.播放声音(refStr声音, str文本);
}

void PlayerComponent::播放声音(SpPlayerComponent& spPlayer可能空, const std::string& refStr声音, const std::string& str文本)
{
	if (spPlayer可能空)
		spPlayer可能空->m_refSession.播放声音(refStr声音, str文本);
}

void PlayerComponent::Send资源(Entity& refEntity)
{
	if (refEntity.m_spPlayer)
		refEntity.m_spPlayer->m_refSession.Send资源();
}

void PlayerComponent::AddComponent(Entity& refEntity, std::weak_ptr<PlayerComponent> wpPlayer, const std::string& strNickName)
{
	if (wpPlayer.expired())
	{
		CHECK_RET_VOID(!strNickName.empty());
		if (!strNickName.empty())
			refEntity.m_spPlayerNickName = std::make_shared<PlayerNickNameComponent, const std::string&>(strNickName);
	}
	else 
	{
		AddComponent(refEntity ,wpPlayer.lock()->m_refSession);
	}
}

void PlayerComponent::AddComponent(Entity& refEntity, PlayerGateSession_Game &refSession)
{
	refEntity.m_spPlayer = std::make_shared<PlayerComponent, PlayerGateSession_Game&>(refSession);
	refEntity.m_spPlayerNickName = std::make_shared<PlayerNickNameComponent, const std::string&>(refSession.NickName());
}
void PlayerComponent::Say(const std::string& str, const SayChannel channel)
{
	m_refSession.Say(str, channel);
}

template<class T>
void PlayerComponent::Send(const std::shared_ptr<PlayerComponent>& spPlayer可能空, const T& ref)
{
	if (spPlayer可能空)
		spPlayer可能空->m_refSession.Send(ref);
}
template void PlayerComponent::Send(const std::shared_ptr<PlayerComponent>& spPlayer可能空, const Msg显示界面& ref);