#include "pch.h"
#include "走Component.h"
#include "Entity.h"
#include "PlayerComponent.h"
#include "AttackComponent.h"
#include "AiCo.h"
#include "MonsterComponent.h"
#include "Space.h"
#include "地堡Component.h"
#include "采集Component.h"
#include "造建筑Component.h"
#include "BuildingComponent.h"
#include "找目标走过去Component.h"
#include "医疗兵Component.h"

void 走Component::AddComponent(Entity& refEntity)
{
	CHECK_VOID(!refEntity.m_sp走);
	refEntity.m_sp走 = std::make_shared<走Component, Entity&>(refEntity);
	//float arrF[] = { refEntity.Pos().x,0,refEntity.Pos().z};
	//int CrowToolAddAgent(float arrF[]);
	//refEntity.m_spAttack->m_idxCrowdAgent = CrowToolAddAgent(arrF);
	//m_mapEntityId[refEntity.m_spAttack->m_idxCrowdAgent] = refEntity.Id;
}

走Component::走Component(Entity& refEntity) :m_refEntity(refEntity)
{
}

bool 走Component::正在走(const Entity& refEntity)
{
	if (!refEntity.m_sp走)
		return false;

	return !refEntity.m_sp走->m_coWalk.Finished();
}

void 走Component::WalkToTarget(SpEntity spTarget)
{
	_ASSERT(!m_cancel);
	_ASSERT(m_coWalk.Finished());
	m_coWalk = AiCo::WalkToTarget(m_refEntity, spTarget, m_cancel);
	m_coWalk.Run();
}

bool 走Component::WalkToTarget(Entity& refThis, SpEntity spTarget)
{
	CHECK_RET_FALSE(refThis.m_sp走);
	refThis.m_sp走->WalkToTarget(spTarget);
	return true;
}

void 走Component::WalkToPos(const Position& posTarget)
{
	_ASSERT(!m_cancel);
	_ASSERT(m_coWalk.Finished());
	m_coWalk = AiCo::WalkToPos(m_refEntity, posTarget, m_cancel);
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
		//PlayerComponent::Say(m_refEntity, "自己阵亡,不能走", SayChannel::系统);

		return;
	}
	//m_coStop = true;

	TryCancel();

	m_coWalk.Run();
	_ASSERT(m_coWalk.Finished());//20240205
	if (m_refEntity.m_spAttack)
	{
		m_refEntity.m_spAttack->TryCancel();
		//_ASSERT(m_refEntity.m_spAttack->m_coAttack.Finished());//20240205
	}
	/*m_coStop = false;*/
	_ASSERT(!m_cancel);
	m_coWalk手动控制 = AiCo::WalkToPos(m_refEntity, posTarget, m_cancel);
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
		if (!m_coWalk.Finished() || !m_coWalk手动控制.Finished())
		{
			LOG(ERROR) << "协程没结束，却提前清空了m_cancel";
			_ASSERT(false);
		}
	}

	_ASSERT(m_coWalk.Finished());//20240205
	//_ASSERT(m_coAttack.Finished());//20240205

}

void 走Component::走进地堡(WpEntity wpEntity地堡)
{
	if (m_refEntity.m_spAttack)
		m_refEntity.m_spAttack->TryCancel();

	if (m_refEntity.m_up找目标走过去)
		m_refEntity.m_up找目标走过去->TryCancel();

	if (m_refEntity.m_up医疗兵)
		m_refEntity.m_up医疗兵->TryCancel();

	if (wpEntity地堡.expired())
		return;

	if (!wpEntity地堡.lock()->m_spBuilding)
		return;

	if (!wpEntity地堡.lock()->m_spBuilding->已造好())
	{
		PlayerComponent::播放声音Buzz(m_refEntity, "地堡还没造好，不能进地堡");
		return;
	}

	if (造建筑Component::正在建造(m_refEntity))
	{
		PlayerComponent::播放声音Buzz(m_refEntity, "正在建造，不能进地堡");
		return;
	}

	if (m_refEntity.m_spAttack)
		m_refEntity.m_spAttack->TryCancel();

	Cancel所有包含走路的协程(m_refEntity);
	_ASSERT(!m_cancel);
	_ASSERT(m_coWalk进地堡.Finished());
	m_coWalk进地堡 = Co走进地堡(wpEntity地堡);
	m_coWalk进地堡.Run();
}

CoTaskBool 走Component::Co走进地堡(WpEntity wpEntity地堡)
{
	KeepCancel kc(m_cancel);

	if (m_refEntity.m_spAttack)
	{
		switch (m_refEntity.m_类型)
		{
		case 工程车:PlayerComponent::播放声音(m_refEntity, "语音/明白女声可爱版", ""); break;
		case 枪兵:PlayerComponent::播放声音(m_refEntity, "语音/明白男声正经版", ""); break;
		default:break;
		}
	}
	while (!wpEntity地堡.expired())
	{
		if (m_refEntity.IsDead())
			co_return false;

		auto spEntity地堡 = wpEntity地堡.lock();
		if (!spEntity地堡->m_sp地堡)
			co_return false;

		if (m_refEntity.DistanceLessEqual(*spEntity地堡, m_refEntity.攻击距离() + BuildingComponent::建筑半边长(*spEntity地堡)))
		{
			spEntity地堡->m_sp地堡->进(m_refEntity.m_refSpace, m_refEntity);

			co_return false;
		}

		_ASSERT(!m_cancel);
		if (co_await AiCo::WalkToTarget(m_refEntity, spEntity地堡, m_cancel, false))
			co_return true;

	}

	co_return false;
}

void 走Component::Cancel所有包含走路的协程(Entity& refEntity, const bool b停止攻击)
{
	if (b停止攻击 && refEntity.m_spAttack)refEntity.m_spAttack->TryCancel();
	if (refEntity.m_sp采集)		refEntity.m_sp采集->m_TaskCancel.TryCancel();
	if (refEntity.m_sp走)		refEntity.m_sp走->TryCancel();
	if (refEntity.m_sp造建筑)	refEntity.m_sp造建筑->TryCancel();
}