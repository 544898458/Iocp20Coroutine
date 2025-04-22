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

void PlayerComponent::Say系统(Entity& refEntity, const std::string& str)
{
	if (refEntity.m_upPlayer)
		refEntity.m_upPlayer->m_refSession.Say系统(str);
}

void PlayerComponent::剧情对话(Space& refSpace, const std::string& refStrNickName,
	const std::string& str头像左, const std::string& str名字左,
	const std::string& str头像右, const std::string& str名字右,
	const std::string& str内容, const bool b显示退出场景按钮)
{
	auto wpPlayerSession = GetPlayerGateSession(refStrNickName);
	Msg剧情对话 msg = { .str头像左 = StrConv::GbkToUtf8(str头像左),
	.str名字左 = StrConv::GbkToUtf8(str名字左),
	.str头像右 = StrConv::GbkToUtf8(str头像右),
	.str名字右 = StrConv::GbkToUtf8(str名字右),
	.str对话内容 = StrConv::GbkToUtf8(str内容),
	.b显示退出场景按钮 = b显示退出场景按钮 };
	
	if (!wpPlayerSession.expired())
		wpPlayerSession.lock()->Send(msg);

	refSpace.GetSpacePlayer(refStrNickName).m_msg上次发给前端的剧情对话 = msg;
}

void PlayerComponent::剧情对话已看完(const std::string& refStrNickName)
{
	auto wpPlayerSession = GetPlayerGateSession(refStrNickName);
	if (!wpPlayerSession.expired())
		wpPlayerSession.lock()->剧情对话已看完();
}

void PlayerComponent::播放声音Buzz(Entity& refEntity, const std::string& str文本) 
{
	if (refEntity.m_upPlayer)
		refEntity.m_upPlayer->m_refSession.播放声音Buzz( str文本);
}
void PlayerComponent::播放声音(Entity& refEntity, const std::string& refStr声音, const std::string& str文本)
{
	if (refEntity.m_upPlayer)
		refEntity.m_upPlayer->m_refSession.播放声音(refStr声音, str文本);
}
void PlayerComponent::播放声音(const std::string& refStrNickName, const std::string& refStr声音, const std::string& str文本)
{
	auto wpPlayerSession = GetPlayerGateSession(refStrNickName);
	if (!wpPlayerSession.expired())
		wpPlayerSession.lock()->播放声音(refStr声音, str文本);
}

void PlayerComponent::播放声音(SpPlayerComponent& spPlayer可能空, const std::string& refStr声音, const std::string& str文本)
{
	if (spPlayer可能空)
		spPlayer可能空->m_refSession.播放声音(refStr声音, str文本);
}

void PlayerComponent::Send资源(Entity& refEntity)
{
	if (refEntity.m_upPlayer)
		refEntity.m_upPlayer->m_refSession.Send资源();
}

void PlayerComponent::AddComponent(Entity& refEntity, std::weak_ptr<PlayerComponent> wpPlayer, const std::string& strNickName)
{
	if (wpPlayer.expired())
	{
		//CHECK_RET_VOID(!strNickName.empty());
		if (!strNickName.empty())//没有玩家昵称说明就是怪（敌人）
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
void PlayerComponent::Send(const std::shared_ptr<PlayerComponent>& spPlayer可能空, const T& ref)
{
	if (spPlayer可能空)
		spPlayer可能空->m_refSession.Send(ref);
}
template void PlayerComponent::Send(const std::shared_ptr<PlayerComponent>& spPlayer可能空, const Msg显示界面& ref);