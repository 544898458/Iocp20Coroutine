#include "pch.h"
#include "AttackComponent.h"
#include "Entity.h"
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
void AttackComponent::AddComponent(Entity& refEntity, const 活动单位类型 类型)
{
	CHECK_VOID(!refEntity.m_spAttack);
	refEntity.m_spAttack = std::make_shared<AttackComponent, Entity&, const 活动单位类型>(refEntity, std::forward<const 活动单位类型&&>(类型));
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

inline AttackComponent::AttackComponent(Entity& refEntity, const 活动单位类型 类型) :m_refEntity(refEntity), m_类型(类型)
{

}

void AttackComponent::TryCancel()
{
	if (m_cancel)
	{
		//LOG(INFO) << "调用m_cancel";
		m_cancel();
	}
	else
	{
		//LOG(INFO) << "m_cancel是空的，没有要取消的协程";
		if (!m_coAttack.Finished() || (m_refEntity.m_spMonster && !m_refEntity.m_spMonster->m_coIdle.Finished()))
		{
			LOG(ERROR) << "协程没结束，却提前清空了m_cancel";
			assert(false);
		}
	}

	//assert(m_coWalk.Finished());//20240205
	assert(m_coAttack.Finished());//20240205

}

void AttackComponent::Update()
{
	if (!m_coAttack.Finished())
		return;//表示不允许打断

	//if (!m_coWalk手动控制.Finished())
	//	return;//表示不允许打断

	if (m_refEntity.IsDead())
		return;

	std::vector<std::pair<int64_t, SpEntity>> vecEnemy;
	std::copy_if(m_refEntity.m_refSpace.m_mapEntity.begin(), m_refEntity.m_refSpace.m_mapEntity.end(), std::back_inserter(vecEnemy), [this](const auto& pair)
		{
			auto& sp = pair.second;
			return sp.get() != &m_refEntity && !sp->IsDead() && sp->IsEnemy(m_refEntity);
		});

	auto iterMin = std::min_element(vecEnemy.begin(), vecEnemy.end(), [this](const auto& pair1, const auto& pair2)
		{
			auto& sp1 = pair1.second;
			auto& sp2 = pair2.second;
			return m_refEntity.DistancePow2(*sp1) < m_refEntity.DistancePow2(*sp2);
		});

	const auto& wpEntity = m_refEntity.m_refSpace.Get最近的Entity(m_refEntity, true, [](const Entity& ref)->bool {return nullptr != ref.m_spDefence; });
	if (!wpEntity.expired())
	{
		Entity& refTarget = *wpEntity.lock();
		if (m_refEntity.DistanceLessEqual(refTarget, 攻击距离()))
		{
			走Component::Cancel所有包含走路的协程(m_refEntity); //TryCancel();

			m_coAttack = CoAttack(wpEntity.lock(), m_cancel);
			m_coAttack.Run();
			return;
		}
		else if (m_refEntity.m_wpOwner.expired() && m_refEntity.DistanceLessEqual(refTarget, m_refEntity.m_f警戒距离) && !走Component::正在走(m_refEntity) && (!m_refEntity.m_sp采集 || m_refEntity.m_sp采集->m_TaskCancel.co.Finished()))
		{
			走Component::Cancel所有包含走路的协程(m_refEntity); //TryCancel();

			//m_coWalk.Run();
			//assert(m_coWalk.Finished());//20240205
			assert(m_coAttack.Finished());//20240205
			/*m_coStop = false;*/
			if (m_refEntity.m_sp采集)
			{
				m_refEntity.m_sp采集->m_TaskCancel.TryCancel();
			}

			走Component::WalkToTarget(m_refEntity, wpEntity.lock());

			return;
		}
	}

	if (!m_refEntity.m_spPlayer && !走Component::正在走(m_refEntity))//怪随机走
	{
		走Component::Cancel所有包含走路的协程(m_refEntity); //TryCancel();
		//assert(m_coWalk.Finished());//20240205
		assert(m_coAttack.Finished());//20240205

		auto posTarget = m_refEntity.m_Pos;
		posTarget.x += std::rand() % 11 - 5;//随机走
		posTarget.z += std::rand() % 11 - 5;
		//m_coWalk = AiCo::WalkToPos(m_refEntity.shared_from_this(), posTarget, m_cancel);
		走Component::WalkToPos(m_refEntity, posTarget);
	}
}


CoTask<int> AttackComponent::CoAttack(WpEntity wpDefencer, FunCancel& cancel)
{
	KeepCancel kc(cancel);

	if (m_refEntity.IsDead())
		co_return 0;//自己死亡

	m_refEntity.BroadcastChangeSkeleAnim("attack");//播放攻击动作

	using namespace std;

	const std::tuple<std::chrono::milliseconds, int> arrWaitHurt[] =
	{	//三段伤害{每段前摇时长，伤害值}
		{300ms,1},
		{50ms,3},
		{50ms,10}
	};

	for (auto wait_hurt : arrWaitHurt)
	{
		if (co_await CoTimer::Wait(std::get<0>(wait_hurt), cancel))//等x秒	前摇
			co_return 0;//协程取消

		if (m_refEntity.IsDead())
			co_return 0;//自己死亡，不再后摇

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
		co_return 0;//协程取消

	if (!m_refEntity.IsDead())
	{
		m_refEntity.BroadcastChangeSkeleAnim("idle");//播放休闲待机动作
	}

	co_return 0;//协程正常退出
}