#include "pch.h"
#include "Entity.h"
#include "GameSvrSession.h"
#include "Space.h"
#include "../CoRoutine/CoTimer.h"
#include "../CoRoutine/CoEvent.h"
#include "GameSvr.h"
#include "AiCo.h"
#include <cmath>
#include "PlayerComponent.h"
#include "AiCo.h"
#include "../IocpNetwork/StrConv.h"
#include "MonsterComponent.h"
#include "AttackComponent.h"
#include "BuildingComponent.h"
#include "PlayerGateSession_Game.h"
#include "采集Component.h"
#include "造活动单位Component.h"
#include "DefenceComponent.h"
#include "地堡Component.h"
#include "走Component.h"
#include "造建筑Component.h"

using namespace std;

Entity::Entity(const Position& pos, Space& space, const std::string& strPrefabName, const std::string& strEntityName) :
	Id((uint64_t)this), m_strPrefabName(strPrefabName), m_refSpace(space), m_Pos(pos), m_strEntityName(strEntityName)
{
}

bool Entity::DistanceLessEqual(const Entity& refEntity, float fDistance)
{
	return this->m_Pos.DistanceLessEqual(refEntity.m_Pos, fDistance);
}

float Entity::DistancePow2(const Entity& refEntity)const
{
	return this->m_Pos.DistancePow2(refEntity.m_Pos);
}
float Entity::Distance(const Entity& refEntity)const
{
	return std::sqrtf(DistancePow2(refEntity));
}

bool Entity::IsDead() const
{
	if (!m_spDefence)
		return false;
	return m_spDefence->IsDead();
}

Entity::~Entity()
{
	LOG(INFO) << Id;
}

//主线程单线程执行
void Entity::Update()
{
	if (m_spAttack)
		m_spAttack->Update();

	if (m_sp地堡)
		m_sp地堡->Update();
}

bool Entity::IsEnemy(const Entity& refEntity)
{
	if (!m_spPlayer && !refEntity.m_spPlayer)
		return false;//都是怪

	if (!m_spPlayer || !refEntity.m_spPlayer)
		return true;//有一个是怪

	//都是玩家单位
	return &m_spPlayer->m_refSession != &refEntity.m_spPlayer->m_refSession;
}

float Entity::攻击距离() const
{
	if (m_spAttack)
		return m_spAttack->m_f攻击距离;

	return 0;
}

void Entity::OnDestroy()
{
	BroadcastLeave();

	//应该用proxy库同意调用下面的，免得忘了
	if (m_spAttack)
		m_spAttack->TryCancel();

	if (m_sp造活动单位)
		m_sp造活动单位->TryCancel(*this);

	if (m_sp采集)
		m_sp采集->m_TaskCancel.TryCancel();

	if (m_sp走)
		m_sp走->TryCancel();

	if (m_sp地堡)
		m_sp地堡->OnDestroy();

	if (m_sp临时阻挡)
		m_sp临时阻挡.reset();

	if (m_sp造建筑)
		m_sp造建筑->TryCancel();

	if (m_cancelDelete)
	{
		LOG(INFO) << "取消删除协程";
		m_cancelDelete();
	}
	else
	{
		LOG(INFO) << "没有删除协程";
	}
}

void Entity::BroadcastLeave()
{
	LOG(INFO) << NickName() << "调用Entity::BroadcastLeave," << Id;
	Broadcast(MsgDelRoleRet(Id));
}

const std::string& Entity::NickName()
{
	if (m_spPlayer)
		return m_spPlayer->m_refSession.NickName();

	if (m_sp资源)
	{
		static const std::string str("资源");
		return str;
	}

	{
		static const std::string str("敌人");
		return str;
	}
}


void Entity::BroadcastEnter()
{
	LOG(INFO) << NickName() << "调用Entity::BroadcastEnter," << Id;
	Broadcast(MsgAddRoleRet(*this));//自己广播给别人
	BroadcastNotifyPos();
	CoEvent<MyEvent::AddEntity>::OnRecvEvent(false, { weak_from_this() });
}

void Entity::BroadcastNotifyPos()
{
	Broadcast(MsgNotifyPos(*this));
}

void Entity::BroadcastChangeSkeleAnim(const std::string& refAniClipName, bool loop)
{
	Broadcast(MsgChangeSkeleAnim(*this, refAniClipName, loop));//播放死亡动作
}

template<class T>
void Entity::Broadcast(const T& msg)
{
	m_refSpace.Broadcast(msg);
}

CoTaskBool Entity::CoDelayDelete()
{
	LOG(INFO) << "开始删除延时自己的协程";
	//assert(!m_cancelDelete);//不可并行
	if (m_cancelDelete)
		co_return false;

	if (m_sp地堡)
		m_sp地堡->OnBeforeDelayDelete();

	using namespace std;
	if (co_await CoTimer::Wait(3s, m_cancelDelete))//服务器主工作线程大循环，每次循环触发一次
	{
		LOG(INFO) << "WaitDelete协程取消了";
		co_return true;
	}

	m_bNeedDelete = true;
	co_return false;
}

template void Entity::Broadcast(const MsgAddRoleRet& msg);
template void Entity::Broadcast(const MsgDelRoleRet& msg);
template void Entity::Broadcast(const MsgEntity描述& msg);
template void Entity::Broadcast(const Msg播放声音& msg);