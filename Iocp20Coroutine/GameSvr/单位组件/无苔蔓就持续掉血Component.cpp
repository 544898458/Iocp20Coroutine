#include "pch.h"
#include "无苔蔓就持续掉血Component.h"
#include "苔蔓Component.h"
#include "Entity.h"
#include "../../CoRoutine/CoTimer.h"
#include "../EntitySystem.h"
#include "BuffComponent.h"
#include "../枚举/BuffId.h"

void 无苔蔓就持续掉血Component::AddComponent(Entity& refEntity)
{
	refEntity.AddComponentOnDestroy(&Entity::m_up无苔蔓就持续掉血,refEntity);
}

无苔蔓就持续掉血Component::无苔蔓就持续掉血Component(Entity& ref) :m_refEntity(ref)
{
	Co反复加持续掉血Buff().RunNew();
}

CoTaskBool 无苔蔓就持续掉血Component::Co反复加持续掉血Buff()
{
	using namespace std;
	while (!co_await CoTimer::Wait(5s, m_funCancel))
	{
		CHECK_CO_RET_FALSE(m_refEntity.m_upBuff);
		auto& ref苔蔓 = *m_wp苔蔓.lock();
		if (!m_refEntity.m_upBuff->已有Buff(离开苔蔓的虫建筑持续扣血))
			m_refEntity.m_upBuff->定时改数值(离开苔蔓的虫建筑持续扣血, 0);
	}
	co_return false;
}

void 无苔蔓就持续掉血Component::OnEntityDestroy(const bool bDestroy)
{
	if (m_funCancel)
	{
		m_funCancel();
		m_funCancel = nullptr;
	}
}
