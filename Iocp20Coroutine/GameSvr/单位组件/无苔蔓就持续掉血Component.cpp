#include "pch.h"
#include "�뿪��̺�ͳ�����ѪComponent.h"
#include "̦��Component.h"
#include "Entity.h"
#include "../../CoRoutine/CoTimer.h"
#include "../EntitySystem.h"

void ��̦���ͳ�����ѪComponent::AddComponent(Entity& refEntity)
{
	LOG_IF(ERROR, refEntity.m_up��̦���ͳ�����Ѫ) << "m_up�뿪��̺�ͳ�����Ѫ";
	refEntity.m_up��̦���ͳ�����Ѫ.reset(new ��̦���ͳ�����ѪComponent(refEntity));
}

��̦���ͳ�����ѪComponent::��̦���ͳ�����ѪComponent(Entity& ref) :m_refEntity(ref)
{
	Co̦������().RunNew();
}

CoTaskBool ��̦���ͳ�����ѪComponent::Co̦������()
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

void ��̦���ͳ�����ѪComponent::TryCancel()
{
	if (m_funCancel)
	{
		m_funCancel();
		m_funCancel = nullptr;
	}
}
