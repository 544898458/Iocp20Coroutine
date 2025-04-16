#include "pch.h"
#include "定时改数值Component.h"
#include "DefenceComponent.h"
#include "Entity.h"
#include "../../CoRoutine/CoTimer.h"

inline 定时改数值Component::定时改数值Component(Entity& ref) :m_refEntity(ref)
{
	using namespace std;
	Co定时改数值(2s, 1).RunNew();
}

CoTaskBool 定时改数值Component::Co定时改数值(std::chrono::system_clock::duration dura间隔, int16_t i16变化)
{
	while (!co_await CoTimer::Wait(dura间隔, m_funCancel))
	{
		CHECK_CO_RET_FALSE(m_refEntity.m_spDefence);
		if(m_refEntity.IsDead())
			co_return false;

		if (m_refEntity.m_spDefence->已满血())
			continue;

		m_refEntity.m_spDefence->m_hp += i16变化;
		m_refEntity.BroadcastNotifyPos();
	}
	co_return false;
}

void 定时改数值Component::AddComponent(Entity& refEntity)
{
	refEntity.m_up定时改数值.reset(new 定时改数值Component(refEntity));
}

void 定时改数值Component::OnDestroy()
{
	if (m_funCancel)
	{
		m_funCancel();
		m_funCancel = nullptr;
	}
}

