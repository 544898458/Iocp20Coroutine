#include "pch.h"
#include "��̦���ͳ�����ѪComponent.h"
#include "̦��Component.h"
#include "Entity.h"
#include "../../CoRoutine/CoTimer.h"
#include "../EntitySystem.h"
#include "BuffComponent.h"
#include "../ö��/BuffId.h"

void ��̦���ͳ�����ѪComponent::AddComponent(Entity& refEntity)
{
	refEntity.AddComponentOnDestroy(&Entity::m_up��̦���ͳ�����Ѫ,refEntity);
}

��̦���ͳ�����ѪComponent::��̦���ͳ�����ѪComponent(Entity& ref) :m_refEntity(ref)
{
	Co�����ӳ�����ѪBuff().RunNew();
}

CoTaskBool ��̦���ͳ�����ѪComponent::Co�����ӳ�����ѪBuff()
{
	using namespace std;
	while (!co_await CoTimer::Wait(5s, m_funCancel))
	{
		CHECK_CO_RET_FALSE(m_refEntity.m_upBuff);
		auto& ref̦�� = *m_wp̦��.lock();
		if (!m_refEntity.m_upBuff->����Buff(�뿪̦���ĳ潨��������Ѫ))
			m_refEntity.m_upBuff->��ʱ����ֵ(�뿪̦���ĳ潨��������Ѫ, 0);
	}
	co_return false;
}

void ��̦���ͳ�����ѪComponent::OnEntityDestroy(const bool bDestroy)
{
	if (m_funCancel)
	{
		m_funCancel();
		m_funCancel = nullptr;
	}
}
