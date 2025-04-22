#include "pch.h"
#include "PlayerComponent.h"
#include "Entity.h"
#include "GameSvrSession.h"
#include "PlayerGateSession_Game.h"
#include "../IocpNetwork/StrConv.h"
#include "PlayerNickNameComponent.h"

std::weak_ptr<PlayerGateSession_Game> GetPlayerGateSession(const std::string& refStrNickName);

void PlayerComponent::Say(Entity& refEntity, const std::string& str, const SayChannel channel)
{
	if (refEntity.m_upPlayer)
		refEntity.m_upPlayer->Say(str, channel);
}

void PlayerComponent::Sayϵͳ(Entity& refEntity, const std::string& str)
{
	if (refEntity.m_upPlayer)
		refEntity.m_upPlayer->m_refSession.Sayϵͳ(str);
}

void PlayerComponent::����Ի�(Space& refSpace, const std::string& refStrNickName,
	const std::string& strͷ����, const std::string& str������,
	const std::string& strͷ����, const std::string& str������,
	const std::string& str����, const bool b��ʾ�˳�������ť)
{
	auto wpPlayerSession = GetPlayerGateSession(refStrNickName);
	Msg����Ի� msg = { .strͷ���� = StrConv::GbkToUtf8(strͷ����),
	.str������ = StrConv::GbkToUtf8(str������),
	.strͷ���� = StrConv::GbkToUtf8(strͷ����),
	.str������ = StrConv::GbkToUtf8(str������),
	.str�Ի����� = StrConv::GbkToUtf8(str����),
	.b��ʾ�˳�������ť = b��ʾ�˳�������ť };
	
	if (!wpPlayerSession.expired())
		wpPlayerSession.lock()->Send(msg);

	refSpace.GetSpacePlayer(refStrNickName).m_msg�ϴη���ǰ�˵ľ���Ի� = msg;
}

void PlayerComponent::����Ի��ѿ���(const std::string& refStrNickName)
{
	auto wpPlayerSession = GetPlayerGateSession(refStrNickName);
	if (!wpPlayerSession.expired())
		wpPlayerSession.lock()->����Ի��ѿ���();
}

void PlayerComponent::��������Buzz(Entity& refEntity, const std::string& str�ı�) 
{
	if (refEntity.m_upPlayer)
		refEntity.m_upPlayer->m_refSession.��������Buzz( str�ı�);
}
void PlayerComponent::��������(Entity& refEntity, const std::string& refStr����, const std::string& str�ı�)
{
	if (refEntity.m_upPlayer)
		refEntity.m_upPlayer->m_refSession.��������(refStr����, str�ı�);
}
void PlayerComponent::��������(const std::string& refStrNickName, const std::string& refStr����, const std::string& str�ı�)
{
	auto wpPlayerSession = GetPlayerGateSession(refStrNickName);
	if (!wpPlayerSession.expired())
		wpPlayerSession.lock()->��������(refStr����, str�ı�);
}

void PlayerComponent::��������(SpPlayerComponent& spPlayer���ܿ�, const std::string& refStr����, const std::string& str�ı�)
{
	if (spPlayer���ܿ�)
		spPlayer���ܿ�->m_refSession.��������(refStr����, str�ı�);
}

void PlayerComponent::Send��Դ(Entity& refEntity)
{
	if (refEntity.m_upPlayer)
		refEntity.m_upPlayer->m_refSession.Send��Դ();
}

void PlayerComponent::AddComponent(Entity& refEntity, std::weak_ptr<PlayerComponent> wpPlayer, const std::string& strNickName)
{
	if (wpPlayer.expired())
	{
		//CHECK_RET_VOID(!strNickName.empty());
		if (!strNickName.empty())//û������ǳ�˵�����ǹ֣����ˣ�
			refEntity.m_upPlayerNickName = std::make_shared<PlayerNickNameComponent, const std::string&>(strNickName);
	}
	else 
	{
		AddComponent(refEntity ,wpPlayer.lock()->m_refSession);
	}
}

void PlayerComponent::AddComponent(Entity& refEntity, PlayerGateSession_Game &refSession)
{
	refEntity.m_upPlayer = std::make_shared<PlayerComponent, PlayerGateSession_Game&>(refSession);
	refEntity.m_upPlayerNickName = std::make_shared<PlayerNickNameComponent, const std::string&>(refSession.NickName());
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
template void PlayerComponent::Send(const std::shared_ptr<PlayerComponent>& spPlayer���ܿ�, const Msg��ʾ����& ref);