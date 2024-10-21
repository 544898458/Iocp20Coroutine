#include "pch.h"
#include "走Component.h"
#include "Entity.h"
#include "PlayerComponent.h"
#include "AttackComponent.h"
#include "AiCo.h"
#include "MonsterComponent.h"

void 走Component::AddComponent(Entity& refEntity)
{
	CHECK_VOID(!refEntity.m_spAttack);
	refEntity.m_sp走 = std::make_shared<走Component, Entity&>(refEntity);
	//float arrF[] = { refEntity.m_Pos.x,0,refEntity.m_Pos.z};
	//int CrowToolAddAgent(float arrF[]);
	//refEntity.m_spAttack->m_idxCrowdAgent = CrowToolAddAgent(arrF);
	//m_mapEntityId[refEntity.m_spAttack->m_idxCrowdAgent] = refEntity.Id;
}

走Component::走Component(Entity& refEntity):m_refEntity(refEntity)
{
}

bool 走Component::正在走(Entity& refEntity)
{
	if (!refEntity.m_sp走)
		return false;

	return !refEntity.m_sp走->m_coWalk.Finished();
}

void 走Component::WalkToTarget(SpEntity spTarget)
{
	m_coWalk = AiCo::WalkToTarget(m_refEntity.shared_from_this(), spTarget, m_cancel);
}

bool 走Component::WalkToTarget(Entity& refThis, SpEntity spTarget)
{
	CHECK_RET_FALSE(refThis.m_sp走);
	refThis.m_sp走->WalkToTarget(spTarget);
	return true;
}

void 走Component::WalkToPos(const Position& posTarget)
{
	m_coWalk = AiCo::WalkToPos(m_refEntity.shared_from_this(), posTarget, m_cancel);
	m_coWalk.Run();//协程离开开始运行（运行到第一个co_await
}

bool 走Component::WalkToPos(Entity& refThis, const Position& posTarget)
{
	CHECK_RET_FALSE(refThis.m_sp走);
	refThis.m_sp走->WalkToPos(posTarget);
	return true;
}

void 走Component::WalkToPos手动控制(const Position& posTarget)
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
	if (m_refEntity.m_spAttack)
	{
		m_refEntity.m_spAttack->TryCancel();
		assert(m_refEntity.m_spAttack->m_coAttack.Finished());//20240205
	}
	/*m_coStop = false;*/
	m_coWalk手动控制 = AiCo::WalkToPos(m_refEntity.shared_from_this(), posTarget, m_cancel);
	m_coWalk手动控制.Run();//协程离开开始运行（运行到第一个co_await
}


void 走Component::TryCancel()
{
	if (m_cancel)
	{
		//LOG(INFO) << "调用m_cancel";
		m_cancel();
	}
	else
	{
		//LOG(INFO) << "m_cancel是空的，没有要取消的协程";
		if (!m_coWalk.Finished() || (m_refEntity.m_spMonster && !m_refEntity.m_spMonster->m_coIdle.Finished()) || !m_coWalk手动控制.Finished())
		{
			LOG(ERROR) << "协程没结束，却提前清空了m_cancel";
			assert(false);
		}
	}

	assert(m_coWalk.Finished());//20240205
	//assert(m_coAttack.Finished());//20240205

}
