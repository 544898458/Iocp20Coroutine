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

inline AttackComponent::AttackComponent(Entity& refEntity, const 活动单位类型 类型) :m_refEntity(refEntity), m_类型(类型)
{

}

void AttackComponent::WalkToPos手动控制(const Position& posTarget)
{
	//void CrowdToolSetMoveTarget(const float* p, const int idx);
	//float arrFloat[] = { posTarget.x,0,posTarget.z };
	//CrowdToolSetMoveTarget(arrFloat, refEntity.m_spAttack->m_idxCrowdAgent);
	//return;
	if (m_refEntity.IsDead())
	{
		if (m_refEntity.m_spPlayer)
			m_refEntity.m_spPlayer->Say("自己阵亡,不能走");

		return;
	}
	//m_coStop = true;

	TryCancel();

	m_coWalk.Run();
	assert(m_coWalk.Finished());//20240205
	assert(m_coAttack.Finished());//20240205
	/*m_coStop = false;*/
	m_coWalk手动控制 = AiCo::WalkToPos(m_refEntity.shared_from_this(), posTarget, m_cancel);
	m_coWalk手动控制.Run();//协程离开开始运行（运行到第一个co_await
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
		if (!m_coWalk.Finished() || !m_coAttack.Finished() || (m_refEntity.m_spMonster && !m_refEntity.m_spMonster->m_coIdle.Finished()) || !m_coWalk手动控制.Finished())
		{
			LOG(ERROR) << "协程没结束，却提前清空了m_cancel";
			assert(false);
		}
	}

	assert(m_coWalk.Finished());//20240205
	assert(m_coAttack.Finished());//20240205

}

void AttackComponent::Update()
{
	if (!m_coAttack.Finished())
		return;//表示不允许打断

	if (!m_coWalk手动控制.Finished())
		return;//表示不允许打断

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
		if (m_refEntity.DistanceLessEqual(*wpEntity.lock(), m_refEntity.m_f攻击距离))
		{
			TryCancel();

			m_coAttack = AiCo::Attack(m_refEntity.shared_from_this(), wpEntity.lock(), m_cancel);
			m_coAttack.Run();
			return;
		}
		else if (m_refEntity.DistanceLessEqual(*wpEntity.lock(), m_refEntity.m_f警戒距离) && m_coWalk.Finished() && (!m_refEntity.m_sp采集 || m_refEntity.m_sp采集->m_TaskCancel.co.Finished()))
		{
			TryCancel();

			//m_coWalk.Run();
			assert(m_coWalk.Finished());//20240205
			assert(m_coAttack.Finished());//20240205
			/*m_coStop = false;*/
			if (m_refEntity.m_sp采集)
			{
				m_refEntity.m_sp采集->m_TaskCancel.TryCancel();
			}
			m_coWalk = AiCo::WalkToTarget(m_refEntity.shared_from_this(), wpEntity.lock(), m_cancel);
			m_coWalk.Run();//协程离开开始运行（运行到第一个co_await
			return;
		}
	}

	if (!m_refEntity.m_spPlayer && m_coWalk.Finished())//怪随机走
	{
		TryCancel();
		assert(m_coWalk.Finished());//20240205
		assert(m_coAttack.Finished());//20240205

		auto posTarget = m_refEntity.m_Pos;
		posTarget.x += std::rand() % 11 - 5;//随机走
		posTarget.z += std::rand() % 11 - 5;
		m_coWalk = AiCo::WalkToPos(m_refEntity.shared_from_this(), posTarget, m_cancel);
		m_coWalk.Run();//协程离开开始运行（运行到第一个co_await
	}
}

