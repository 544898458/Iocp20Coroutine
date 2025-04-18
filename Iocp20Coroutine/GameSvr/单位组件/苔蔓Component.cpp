#include "pch.h"
#include "苔蔓Component.h"
#include "AoiComponent.h"
#include "Entity.h"
#include "../../CoRoutine/CoTimer.h"
#include "../EntitySystem.h"

void 苔蔓Component::AddComponent(Entity& refEntity)
{
	LOG_IF(ERROR, refEntity.m_up苔蔓) << "m_up苔蔓";
	refEntity.m_up苔蔓.reset(new 苔蔓Component(refEntity));
}

苔蔓Component::苔蔓Component(Entity& ref) :m_refEntity(ref)
{
	Co萎缩消亡().RunNew();
	Co给周围加Buff().RunNew();
}

void 苔蔓Component::TryCancel()
{
	if (m_funCancel萎缩消亡)
	{
		m_funCancel萎缩消亡();
		m_funCancel萎缩消亡 = nullptr;
	}

	if (m_funCancel给周围加Buff)
	{
		m_funCancel给周围加Buff();
        m_funCancel给周围加Buff = nullptr;
	}
}

CoTaskBool 苔蔓Component::Co萎缩消亡()
{
	using namespace std;
	while (!co_await CoTimer::Wait(20s, m_funCancel萎缩消亡))
	{
		if (!m_wp附着建筑.expired())
			continue;

		m_i16半径 -= 2;
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
	//每隔3秒给周围加5秒的Buff
	using namespace std;
	while (!co_await CoTimer::Wait(3s, m_funCancel萎缩消亡))
	{
		if (m_refEntity.IsDead())
			co_return false;

		CHECK_CO_RET_FALSE(m_refEntity.m_upAoi);
		for (auto& [_, wp] : m_refEntity.m_upAoi->m_map我能看到的)
		{
			CHECK_WP_CO_RET_FALSE(wp);
			auto& refEntity = *wp.lock();
			if (!m_refEntity.DistanceLessEqual(refEntity, m_i16半径))
				continue;

			if (!单位::Is虫(refEntity.m_类型))
				continue;
			
			//CHECK_CO_RET_FALSE(refEntity.m_upBuff->AddBuff(m_i16半径, 5s, m_funCancel));
		}
	}
	co_return false;
}
