#include "pch.h"
#include "离开菌毯就持续掉血Component.h"
#include "苔蔓Component.h"
#include "Entity.h"
#include "../../CoRoutine/CoTimer.h"
#include "../EntitySystem.h"

void 无苔蔓就持续掉血Component::AddComponent(Entity& refEntity)
{
	LOG_IF(ERROR, refEntity.m_up无苔蔓就持续掉血) << "m_up离开菌毯就持续掉血";
	refEntity.m_up无苔蔓就持续掉血.reset(new 无苔蔓就持续掉血Component(refEntity));
}

无苔蔓就持续掉血Component::无苔蔓就持续掉血Component(Entity& ref) :m_refEntity(ref)
{
	Co苔蔓扩张().RunNew();
}

CoTaskBool 无苔蔓就持续掉血Component::Co苔蔓扩张()
{
	using namespace std;
	while (!co_await CoTimer::Wait(5s, m_funCancel))
	{
		CHECK_WP_CO_RET_FALSE(m_wp苔蔓);
		auto& ref苔蔓 = *m_wp苔蔓.lock();

		CHECK_CO_RET_FALSE(ref苔蔓.m_up苔蔓);

		ref苔蔓.m_up苔蔓->m_i16半径 += 2;
		EntitySystem::BroadcastEntity苔蔓半径(ref苔蔓);

		if (ref苔蔓.m_up苔蔓->m_i16半径 > 苔蔓Component::MAX半径)
		{
			co_return false;
		}
	}
	co_return false;
}

void 无苔蔓就持续掉血Component::TryCancel()
{
	if (m_funCancel)
	{
		m_funCancel();
		m_funCancel = nullptr;
	}
}
