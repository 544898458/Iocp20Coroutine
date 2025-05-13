#include "pch.h"
#include "苔蔓Component.h"
#include "AoiComponent.h"
#include "BuffComponent.h"
#include "Entity.h"
#include "DefenceComponent.h"
#include "数值Component.h"
#include "../../CoRoutine/CoTimer.h"
#include "../EntitySystem.h"
#include "../枚举/BuffId.h"
#include "../枚举/属性类型.h"

void 苔蔓Component::AddComponent(Entity& refEntity)
{
	refEntity.AddComponentOnDestroy(&Entity::m_up苔蔓, refEntity);
}

苔蔓Component::苔蔓Component(Entity& ref) :m_refEntity(ref)
{
	Co萎缩消亡().RunNew();
	Co给周围加Buff().RunNew();
}

void 苔蔓Component::OnEntityDestroy(const bool bDestroy)
{
	m_funCancel萎缩消亡.TryCancel();
	m_funCancel给周围加Buff.TryCancel();
}

bool 苔蔓Component::在半径内(const Position& pos) const
{
	return m_refEntity.Pos().DistanceLessEqual(pos, m_i16半径);
}

CoTaskBool 苔蔓Component::Co萎缩消亡()
{
	using namespace std;
	while (!co_await CoTimer::Wait(10s, m_funCancel萎缩消亡))
	{
		if (!m_wp附着建筑.expired())
			continue;

		m_i16半径 -= 1;
		EntitySystem::BroadcastEntity苔蔓半径(m_refEntity);

		if (m_i16半径 < 0)
		{
			m_refEntity.DelayDelete();
			co_return false;
		}
	}
	co_return false;
}


CoTaskBool 苔蔓Component::Co给周围加Buff()
{
	using namespace std;
	while (!co_await CoTimer::Wait(1000ms, m_funCancel给周围加Buff))
	{
		if (m_refEntity.IsDead())
			co_return false;

		CHECK_CO_RET_FALSE(m_refEntity.m_upAoi);
		for (auto& [_, wp] : m_refEntity.m_upAoi->m_map我能看到的)
		{
			CHECK_WP_CO_RET_FALSE(wp);
			auto& refEntity = *wp.lock();
			if (!EntitySystem::Is活动单位建筑怪(refEntity))
				continue;

			if (!m_refEntity.DistanceLessEqual(refEntity, m_i16半径))
				continue;


			if (!refEntity.m_upBuff)
				continue;

			if (!refEntity.m_upDefence)
				continue;

			if (单位::Is虫(refEntity.m_类型) && !m_refEntity.IsEnemy(refEntity))//只给友方虫族单位加速度
			{
				refEntity.m_upBuff->加属性(苔蔓给虫单位加移动速度);
				refEntity.m_upBuff->删Buff(离开苔蔓的虫建筑持续扣血);
				数值Component::改变(refEntity, 生命, 1);
			}
			else if (EntitySystem::Is建筑(refEntity))//给人类建筑扣血（不分敌我）
			{
				refEntity.m_upDefence->受伤(1, m_refEntity.Id);
			}
		}
	}
	co_return false;
}

bool 苔蔓Component::扩张结束()const
{
	return m_i16半径 >= MAX半径;
}