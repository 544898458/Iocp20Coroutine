#include "StdAfx.h"
#include "Entity.h"
#include "GameSvrSession.h"
#include "Space.h"
#include "../CoRoutine/CoTimer.h"
#include "MyServer.h"
#include "AiCo.h"
#include <cmath>
#include "PlayerComponent.h"
#include "AiCo.h"
#include "../IocpNetwork/StrConv.h"
#include "MonsterComponent.h"
#include "AttackComponent.h"

using namespace std;

Entity::Entity(const Position& pos, Space& space, const std::string& strPrefabName) :
	Id((uint64_t)this), m_strPrefabName(strPrefabName), m_space(space), m_Pos(pos)
{
}

const float fExponent = 2.0f;
bool Entity::DistanceLessEqual(const Entity& refEntity, float fDistance)
{
	return this->DistancePow2(refEntity) <= std::pow(fDistance, fExponent);
}

float Entity::DistancePow2(const Entity& refEntity)const
{
	return std::pow(this->m_Pos.x - refEntity.m_Pos.x, fExponent) + std::pow(this->m_Pos.z - refEntity.m_Pos.z, fExponent);
}
void Entity::Hurt(int hp)
{
	CHECK_GE(hp, 0);
	if (IsDead())
		return;

	this->m_hp -= hp;

	this->Broadcast(MsgNotifyPos(*this));
	if (IsDead())
	{
		this->Broadcast(MsgChangeSkeleAnim(*this, "died", false));//播放死亡动作
		m_coWaitDelete = AiCo::WaitDelete(shared_from_this(), m_cancelDelete);
		m_coWaitDelete.Run();
	}
}

//主线程单线程执行
void Entity::Update()
{
	if (m_spAttack)
		m_spAttack->Update(*this);
}

bool Entity::IsEnemy(const Entity& refEntity)
{
	if (!m_spPlayer && !refEntity.m_spPlayer)
		return false;//都是怪

	if (!m_spPlayer || !refEntity.m_spPlayer)
		return true;//有一个是怪

	//都是玩家单位
	return m_spPlayer->m_pSession != refEntity.m_spPlayer->m_pSession;
}


void Entity::OnDestroy()
{
	LOG(INFO) << "调用Entity::OnDestroy";
	if( m_spAttack)
		m_spAttack->TryCancel(*this);

	if (m_cancelDelete)
		m_cancelDelete();
}

const std::string& Entity::NickName()
{
	if (m_spPlayer)
		return m_spPlayer->m_pSession->m_nickName;

	static const std::string str怪("怪");
	return str怪;
}


void Entity::BroadcastEnter()
{
	Broadcast(MsgAddRoleRet((uint64_t)this, StrConv::GbkToUtf8(NickName()), m_strPrefabName));//自己广播给别人
	Broadcast(MsgNotifyPos(*this));
}

template<class T>
void Entity::Broadcast(const T& msg)
{
	m_space.m_pServer->m_Sessions.Broadcast(msg);
}

template void Entity::Broadcast(const MsgAddRoleRet& msg);
template void Entity::Broadcast(const MsgDelRoleRet& msg);