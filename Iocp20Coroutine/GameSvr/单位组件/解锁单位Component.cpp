#pragma once
#include "pch.h"
#include "解锁单位Component.h"
#include "造活动单位Component.h"
#include "../Entity.h"
#include "../../CoRoutine/CoTimer.h"
#include "../Space.h"
#include "../EntitySystem.h"

解锁单位Component::解锁单位Component(Entity& ref) :m_refEntity(ref)
{
}

void 解锁单位Component::AddComponent(Entity& refEntity)
{
	refEntity.m_up解锁单位.reset(new 解锁单位Component(refEntity));
}

void 解锁单位Component::TryCancel()
{
	if (m_cancel解锁单位)
		m_cancel解锁单位();
}


void 解锁单位Component::解锁单位(const 单位类型 类型)
{
	auto& spacePlayer = m_refEntity.m_refSpace.GetSpacePlayer(m_refEntity);
	if (!spacePlayer.开始解锁单位(类型, m_refEntity))
		return;

	Co解锁单位(类型).RunNew();
}

CoTaskBool 解锁单位Component::Co解锁单位(const 单位类型 类型)
{
	const int MAX进度 = 10;
	for (int i = 0; i < MAX进度; ++i)
	{
		using namespace std;
		if (co_await CoTimer::Wait(1000ms, m_cancel解锁单位))
			co_return false;

		EntitySystem::BroadcastEntity描述(m_refEntity, std::format("解锁进度{0}/{1}", i + 1, MAX进度));
	}

	EntitySystem::BroadcastEntity描述(m_refEntity, "");
	auto& spacePlayer = m_refEntity.m_refSpace.GetSpacePlayer(m_refEntity);
	spacePlayer.解锁完成(类型, m_refEntity);
	co_return true;
}