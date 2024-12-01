#include "pch.h"
#include "AttackComponent.h"
#include "Entity.h"
#include "EntitySystem.h"
#include "PlayerComponent.h"
#include "MonsterComponent.h"
#include "../IocpNetwork/StrConv.h"
#include "GameSvrSession.h"
#include "AiCo.h"
#include "Space.h"
#include "PlayerGateSession_Game.h"
#include "采集Component.h"
#include "走Component.h"
#include "../CoRoutine/CoTimer.h"
#include "DefenceComponent.h"

extern std::unordered_map<int, uint64_t> m_mapEntityId;
void AttackComponent::AddComponent(Entity& refEntity, const 活动单位类型 类型, const float f攻击距离)
{
	CHECK_VOID(!refEntity.m_spAttack);
	refEntity.m_spAttack = std::make_shared<AttackComponent, Entity&, const 活动单位类型>(refEntity, std::forward<const 活动单位类型&&>(类型));
	refEntity.m_spAttack->m_f攻击距离 = f攻击距离;
	//float arrF[] = { refEntity.m_Pos.x,0,refEntity.m_Pos.z};
	//int CrowToolAddAgent(float arrF[]);
	//refEntity.m_spAttack->m_idxCrowdAgent = CrowToolAddAgent(arrF);
	//m_mapEntityId[refEntity.m_spAttack->m_idxCrowdAgent] = refEntity.Id;
}

float AttackComponent::攻击距离() const
{
	if (m_refEntity.m_wpOwner.expired())
		return m_f攻击距离;

	auto spOwner = m_refEntity.m_wpOwner.lock();
	if (!spOwner->m_spAttack)
		return m_f攻击距离;

	return spOwner->m_spAttack->m_f攻击距离 + m_f攻击距离;
}

Position 怪物闲逛(const Position &refOld)
{
	auto posTarget = refOld;
	posTarget.x += std::rand() % 11 - 5;//随机走
	posTarget.z += std::rand() % 11 - 5;
	return posTarget;
}

AttackComponent::AttackComponent(Entity& refEntity, const 活动单位类型 类型) :
	m_refEntity(refEntity), 
	m_类型(类型),
	m_fun空闲走向此处(怪物闲逛)
{
}

void AttackComponent::TryCancel(const bool bDestroy)
{
	if (m_cancelAttack)
	{
		//LOG(INFO) << "调用m_cancel";
		m_cancelAttack();
	}

	if(bDestroy)
		m_TaskCancel.TryCancel();
}

void AttackComponent::Update()
{
	if (m_refEntity.IsDead())
		return;

	m_TaskCancel.TryRun(Co(m_TaskCancel.cancel));
	if (!m_TaskCancel.co.Finished())
		return;//正在攻击或走向攻击位置

	if (!m_refEntity.m_spPlayer && !走Component::正在走(m_refEntity))//怪随机走
	{
		走Component::Cancel所有包含走路的协程(m_refEntity); //TryCancel();
		//assert(m_coWalk.Finished());//20240205
		//assert(m_coAttack.Finished());//20240205


		auto posTarget = m_fun空闲走向此处(m_refEntity.m_Pos);
		//m_coWalk = AiCo::WalkToPos(m_refEntity.shared_from_this(), posTarget, m_cancel);
		走Component::WalkToPos(m_refEntity, posTarget);
		//co_await AiCo::WalkToPos(m_refEntity, posTarget, funCancel);
		return ;
	}
}

CoTaskBool AttackComponent::Co(FunCancel &funCancel)
{
	KeepCancel kc(funCancel);
	while (true) 
	{
		//if (co_await CoTimer::WaitNextUpdate(funCancel))
		//	co_return true;

		//if (!m_coAttack.Finished())
		//	continue;//表示不允许打断

		if (m_refEntity.m_sp走 && !m_refEntity.m_sp走->m_coWalk手动控制.Finished())
			co_return false;//表示不允许打断

		if (m_refEntity.IsDead())
			co_return false;

		const auto& wpEntity = m_refEntity.m_refSpace.Get最近的Entity(m_refEntity, true, [](const Entity& ref)->bool {return nullptr != ref.m_spDefence; });
		if (!wpEntity.expired())
		{
			Entity& refTarget = *wpEntity.lock();
			if (m_refEntity.DistanceLessEqual(refTarget, 攻击距离()))
			{
				走Component::Cancel所有包含走路的协程(m_refEntity); //TryCancel();

				if (co_await CoAttack(wpEntity.lock(), m_cancelAttack))
					co_return true;

				continue;
			}
			else if (m_refEntity.m_wpOwner.expired() && m_refEntity.DistanceLessEqual(refTarget, m_refEntity.m_f警戒距离) &&
				//!走Component::正在走(m_refEntity) && 
				(!m_refEntity.m_sp采集 || m_refEntity.m_sp采集->m_TaskCancel.co.Finished()))
			{
				走Component::Cancel所有包含走路的协程(m_refEntity); //TryCancel();

				if (m_refEntity.m_sp采集)
				{
					m_refEntity.m_sp采集->m_TaskCancel.TryCancel();
				}

				//走Component::WalkToTarget(m_refEntity, wpEntity.lock());
				if (co_await AiCo::WalkToTarget(m_refEntity, wpEntity.lock(), funCancel))
					co_return true;

				continue;
			}
		}
		co_return false;
	}
	co_return false;
}


CoTaskBool AttackComponent::CoAttack(WpEntity wpDefencer, FunCancel& cancel)
{
	KeepCancel kc(cancel);

	if (m_refEntity.IsDead())
		co_return false;//自己死亡

	m_refEntity.BroadcastChangeSkeleAnim("attack");//播放攻击动作

	using namespace std;

	const std::tuple<std::chrono::milliseconds, int> arrWaitHurt[] =
	{	//三段伤害{每段前摇时长，伤害值}
		{300ms,1},
		{50ms,3},
		{50ms,5}
	};

	for (auto wait_hurt : arrWaitHurt)
	{
		if (co_await CoTimer::Wait(std::get<0>(wait_hurt), cancel))//等x秒	前摇
			co_return true;//协程取消

		if (m_refEntity.IsDead())
			co_return false;//自己死亡，不再后摇

		if (wpDefencer.expired())
			break;//要执行后摇

		auto spDefencer = wpDefencer.lock();
		if (spDefencer->IsDead())
			break;//要执行后摇

		if (!m_refEntity.DistanceLessEqual(*spDefencer, 攻击距离()))
			break;//要执行后摇

		if (!spDefencer->m_spDefence)
			break;//目标打不了

		spDefencer->m_spDefence->受伤(std::get<1>(wait_hurt));//第n次让对方伤1
	}

	if (co_await CoTimer::Wait(1000ms, cancel))//等3秒	后摇
		co_return true;//协程取消

	if (!m_refEntity.IsDead())
	{
		EntitySystem::BroadcastChangeSkeleAnimIdle(m_refEntity);//播放休闲待机动作
	}

	co_return false;//协程正常退出
}