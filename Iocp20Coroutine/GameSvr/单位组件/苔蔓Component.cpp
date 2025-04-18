#include "pch.h"
#include "̦��Component.h"
#include "AoiComponent.h"
#include "Entity.h"
#include "../../CoRoutine/CoTimer.h"
#include "../EntitySystem.h"

void ̦��Component::AddComponent(Entity& refEntity)
{
	LOG_IF(ERROR, refEntity.m_up̦��) << "m_up̦��";
	refEntity.m_up̦��.reset(new ̦��Component(refEntity));
}

̦��Component::̦��Component(Entity& ref) :m_refEntity(ref)
{
	Coή������().RunNew();
	Co����Χ��Buff().RunNew();
}

void ̦��Component::TryCancel()
{
	if (m_funCancelή������)
	{
		m_funCancelή������();
		m_funCancelή������ = nullptr;
	}

	if (m_funCancel����Χ��Buff)
	{
		m_funCancel����Χ��Buff();
        m_funCancel����Χ��Buff = nullptr;
	}
}

CoTaskBool ̦��Component::Coή������()
{
	using namespace std;
	while (!co_await CoTimer::Wait(20s, m_funCancelή������))
	{
		if (!m_wp���Ž���.expired())
			continue;

		m_i16�뾶 -= 2;
		EntitySystem::BroadcastEntity̦���뾶(m_refEntity);

		if (m_i16�뾶 < 0)
		{
			m_refEntity.DelayDelete();
			co_return false;
		}
	}
	co_return false;
}


CoTaskBool ̦��Component::Co����Χ��Buff()
{
	//ÿ��3�����Χ��5���Buff
	using namespace std;
	while (!co_await CoTimer::Wait(3s, m_funCancelή������))
	{
		if (m_refEntity.IsDead())
			co_return false;

		CHECK_CO_RET_FALSE(m_refEntity.m_upAoi);
		for (auto& [_, wp] : m_refEntity.m_upAoi->m_map���ܿ�����)
		{
			CHECK_WP_CO_RET_FALSE(wp);
			auto& refEntity = *wp.lock();
			if (!m_refEntity.DistanceLessEqual(refEntity, m_i16�뾶))
				continue;

			if (!��λ::Is��(refEntity.m_����))
				continue;
			
			//CHECK_CO_RET_FALSE(refEntity.m_upBuff->AddBuff(m_i16�뾶, 5s, m_funCancel));
		}
	}
	co_return false;
}
