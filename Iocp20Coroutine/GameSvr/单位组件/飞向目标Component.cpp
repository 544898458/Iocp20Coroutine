#include "pch.h"
#include "飞向目标Component.h"
#include "DefenceComponent.h"
#include "BuildingComponent.h"
#include "AttackComponent.h"
#include "../Entity.h"
#include "../../CoRoutine/CoTimer.h"
#include "../Space.h"
#include "../EntitySystem.h"
void 飞向目标Component::AddComponent(Entity& refEntity, const Position& pos起始点, const Position& pos方向, const float f最远距离, const uint64_t idAttacker)
{
	refEntity.AddComponentOnDestroy(&Entity::m_up飞向目标, refEntity, pos起始点, pos方向, f最远距离, idAttacker);
}

飞向目标Component::飞向目标Component(Entity& ref, const Position& pos起始点, const Position& vec方向, const float f最远距离, const uint64_t idAttacker) :
	m_refEntity(ref),
	m_pos起始点(pos起始点),
	m_vec方向(vec方向),
	m_f最远距离(f最远距离),
	m_idAttacker(idAttacker),
	m_funCancel("飞向目标Component")
{
	Co飞向目标遇敌爆炸().RunNew();
}

void 飞向目标Component::OnEntityDestroy(const bool bDestroy)
{
	m_funCancel.TryCancel();
}

CoTaskBool 飞向目标Component::Co飞向目标遇敌爆炸()
{
	单位::战斗配置 配置;
	CHECK_CO_RET_FALSE(单位::Find战斗配置(m_refEntity.m_类型, 配置));

	using namespace std;

	float f首次移动倍数 = 3.f;
	while (!co_await CoTimer::WaitNextUpdate(m_funCancel))
	{
		auto wp = m_refEntity.Get最近的Entity(Entity::敌方, [this](const Entity& ref)->bool {return EntitySystem::Is空地能打(m_refEntity.m_类型, ref.m_类型);});
		if (!wp.expired())
		{
			auto &ref目标 = *wp.lock();
			const auto f建筑半边长 = BuildingComponent::建筑半边长(ref目标);
			if (ref目标.m_upDefence && ref目标.Pos().DistanceLessEqual(m_refEntity.Pos(), 1 + f建筑半边长))
			{
				EntitySystem::Broadcast播放声音(m_refEntity, 配置.str攻击音效);
				ref目标.m_upDefence->受伤(EntitySystem::升级后攻击(m_refEntity), m_refEntity);
				m_refEntity.DelayDelete(0s);
				co_return false;
			}
		}

		if (!m_refEntity.Pos().DistanceLessEqual(m_pos起始点, m_f最远距离))//超过射程
		{
			m_refEntity.DelayDelete(0s);
			co_return false;
		}

		const auto posNew = m_refEntity.Pos() + m_vec方向 * 配置.f每帧移动距离 * f首次移动倍数;
		f首次移动倍数 = 1.f;
		//if (!m_refEntity.m_refSpace.CrowdTool可走直线(posNew, m_refEntity.Pos()))//撞墙
		//{
		//	m_refEntity.DelayDelete();
		//	co_return false;
		//}

		m_refEntity.SetPos(posNew);
		m_refEntity.BroadcastNotifyPos();
	}
	m_refEntity.DelayDelete(0s);
	co_return true;
}