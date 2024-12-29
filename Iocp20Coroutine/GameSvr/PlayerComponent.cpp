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

void PlayerComponent::Sayϵͳ(const std::shared_ptr<PlayerComponent>& spPlayer���ܿ�, const std::string& str)
{
	if (spPlayer���ܿ�)
		spPlayer���ܿ�->m_refSession.Sayϵͳ(str);
}

void PlayerComponent::��������(Entity& refEntity, const std::string& refStr����, const std::string& str�ı�)
{
	if (refEntity.m_spPlayer)
		refEntity.m_spPlayer->m_refSession.��������(refStr����, str�ı�);
}

void ��������(SpPlayerComponent& spPlayer���ܿ�, const std::string& refStr����, const std::string& str�ı�)
{
	if (spPlayer���ܿ�)
		spPlayer���ܿ�->m_refSession.��������(refStr����, str�ı�);
}

void PlayerComponent::Send��Դ(Entity& refEntity)
{
	if (refEntity.m_spPlayer)
		refEntity.m_spPlayer->m_refSession.Send��Դ();
}

void PlayerComponent::AddComponent(Entity& refEntity, std::weak_ptr<PlayerComponent> wpPlayer, const std::string& strNickName)
{
	if (!wpPlayer.expired())
		refEntity.m_spPlayer = std::make_shared<PlayerComponent, PlayerGateSession_Game&>(wpPlayer.lock()->m_refSession);

	CHECK_RET_VOID(!strNickName.empty());
	if (!strNickName.empty())
		refEntity.m_spPlayerNickName = std::make_shared<PlayerNickNameComponent, const std::string&>(strNickName);
}

void PlayerComponent::Say(const std::string& str, const SayChannel channel)
{
	m_refSession.Say(str, channel);
}

template<class T>
void PlayerComponent::Send(const std::shared_ptr<PlayerComponent>& spPlayer���ܿ�, const T& ref)
{
	if (spPlayer���ܿ�)
		spPlayer���ܿ�->m_refSession.Send(ref);
}