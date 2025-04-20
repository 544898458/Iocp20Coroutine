#include "pch.h"
#include "̦������Component.h"
#include "̦��Component.h"
#include "Entity.h"
#include "../../CoRoutine/CoTimer.h"
#include "../EntitySystem.h"

void ̦������Component::AddComponent(Entity& refEntity)
{
	LOG_IF(ERROR, refEntity.m_up̦������) << "m_upm_up̦������";
	refEntity.m_up̦������.reset(new ̦������Component(refEntity));
}

̦������Component::̦������Component(Entity& ref) :m_refEntity(ref)
{
	Co�����ӳ�����ѪBuff().RunNew();
}

CoTaskBool ̦������Component::Co�����ӳ�����ѪBuff()
{
	using namespace std;
	while (!co_await CoTimer::Wait(5s, m_funCancel))
	{
		CHECK_WP_CO_RET_FALSE(m_wp̦��);
		auto& ref̦�� = *m_wp̦��.lock();

		CHECK_CO_RET_FALSE(ref̦��.m_up̦��);

		ref̦��.m_up̦��->m_i16�뾶 += 2;
		EntitySystem::BroadcastEntity̦���뾶(ref̦��);

		if (ref̦��.m_up̦��->m_i16�뾶 > ̦��Component::MAX�뾶)
		{
			co_return false;
		}
	}
	co_return false;
}

void ̦������Component::TryCancel()
{
	if (m_funCancel)
	{
		m_funCancel();
		m_funCancel = nullptr;
	}
}
