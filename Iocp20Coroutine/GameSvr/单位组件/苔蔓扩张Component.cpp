#include "pch.h"
#include "苔蔓扩张Component.h"
#include "苔蔓Component.h"
#include "Entity.h"
#include "../../CoRoutine/CoTimer.h"
#include "../EntitySystem.h"

void 苔蔓扩张Component::AddComponent(Entity& refEntity)
{
	refEntity.AddComponentOnDestroy(&Entity::m_up苔蔓扩张, refEntity);
}

苔蔓扩张Component::苔蔓扩张Component(Entity& ref) :m_refEntity(ref), m_funCancel("苔蔓扩张Component")
{
	Co反复加持续掉血Buff().RunNew();
}

CoTaskBool 苔蔓扩张Component::Co反复加持续掉血Buff()
{
	using namespace std;
	while (!co_await CoTimer::Wait(5s, m_funCancel))
	{
		CHECK_WP_CO_RET_FALSE(m_wp苔蔓);
		auto& ref苔蔓 = *m_wp苔蔓.lock();

		CHECK_CO_RET_FALSE(ref苔蔓.m_up苔蔓);

		ref苔蔓.m_up苔蔓->m_i16半径 += 1;
		EntitySystem::BroadcastEntity苔蔓半径(ref苔蔓);

		if (ref苔蔓.m_up苔蔓->m_i16半径 >= 苔蔓Component::MAX半径)
		{
			co_return false;
		}
	}
	co_return false;
}

void 苔蔓扩张Component::OnEntityDestroy(const bool bDestroy)
{
	m_funCancel.TryCancel();
}
