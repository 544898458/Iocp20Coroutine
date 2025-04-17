#include "pch.h"
#include "̦��Component.h"
#include "Entity.h"
#include "../../CoRoutine/CoTimer.h"

void ̦��Component::AddComponent(Entity& refEntity)
{
	LOG_IF(ERROR, refEntity.m_up̦��) << "m_up̦��";
	refEntity.m_up̦��.reset(new ̦��Component(refEntity));
}

void ̦��Component::TryCancel()
{
	if (m_funCancel)
	{
		m_funCancel();
		m_funCancel = nullptr;
	}
}

̦��Component::̦��Component(Entity& ref):m_refEntity(ref)
{
	if (!ref.m_spAttack)
	{
		LOG(ERROR) << "m_spAttack";
		return;
	}
	Co̦������().RunNew();
}

CoTaskBool ̦��Component::Co̦������()
{
	using namespace std;
	while (!co_await CoTimer::Wait(20s, m_funCancel))
	{
		if (m_wp���Ž���.expired())
			continue;

		m_i16�뾶 += 2;
		if (m_i16�뾶 < 0)
		{
			m_refEntity.DelayDelete();
			co_return false;
		}
	}
	co_return false;
}
