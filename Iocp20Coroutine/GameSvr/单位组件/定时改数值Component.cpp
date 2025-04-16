#include "pch.h"
#include "��ʱ����ֵComponent.h"
#include "DefenceComponent.h"
#include "Entity.h"
#include "../../CoRoutine/CoTimer.h"

inline ��ʱ����ֵComponent::��ʱ����ֵComponent(Entity& ref) :m_refEntity(ref)
{
	using namespace std;
	Co��ʱ����ֵ(2s, 1).RunNew();
}

CoTaskBool ��ʱ����ֵComponent::Co��ʱ����ֵ(std::chrono::system_clock::duration dura���, int16_t i16�仯)
{
	while (!co_await CoTimer::Wait(dura���, m_funCancel))
	{
		CHECK_CO_RET_FALSE(m_refEntity.m_spDefence);
		if(m_refEntity.IsDead())
			co_return false;

		if (m_refEntity.m_spDefence->����Ѫ())
			continue;

		m_refEntity.m_spDefence->m_hp += i16�仯;
		m_refEntity.BroadcastNotifyPos();
	}
	co_return false;
}

void ��ʱ����ֵComponent::AddComponent(Entity& refEntity)
{
	refEntity.m_up��ʱ����ֵ.reset(new ��ʱ����ֵComponent(refEntity));
}

void ��ʱ����ֵComponent::OnDestroy()
{
	if (m_funCancel)
	{
		m_funCancel();
		m_funCancel = nullptr;
	}
}

