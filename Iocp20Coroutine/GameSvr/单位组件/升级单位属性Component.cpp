#pragma once
#include "pch.h"
#include "升级单位属性Component.h"
#include "造活动单位Component.h"
#include "BuildingComponent.h"
#include "../Entity.h"
#include "../../CoRoutine/CoTimer.h"
#include "../Space.h"
#include "../EntitySystem.h"
#include "../PlayerGateSession_Game.h"

升级单位属性Component::升级单位属性Component(Entity& ref) :m_refEntity(ref)
{
}

void 升级单位属性Component::AddComponent(Entity& refEntity)
{
	refEntity.m_up升级单位属性.reset(new 升级单位属性Component(refEntity));
}

void 升级单位属性Component::TryCancel()
{
	if (m_cancel升级单位属性)
		m_cancel升级单位属性();
}


void 升级单位属性Component::升级(const 单位类型 单位, const 单位属性类型 属性)
{
	CHECK_RET_VOID(m_refEntity.m_spBuilding);
	if (!m_refEntity.m_spBuilding->已造好())
	{
		//播放声音
        PlayerGateSession_Game::播放声音Buzz(EntitySystem::GetNickName(m_refEntity), "还没造好建筑");
		return;
	}

	auto& spacePlayer = m_refEntity.m_refSpace.GetSpacePlayer(m_refEntity);
	if (!spacePlayer.开始升级单位属性(单位, 属性, m_refEntity))
		return;

	Co升级(单位, 属性).RunNew();
}

CoTaskBool 升级单位属性Component::Co升级(const 单位类型 单位, const 单位属性类型 属性)
{
	const int MAX进度 = 10;
	for (int i = 0; i < MAX进度; ++i)
	{
		using namespace std;
		if (co_await CoTimer::Wait(1000ms, m_cancel升级单位属性))
			co_return false;

		EntitySystem::BroadcastEntity描述(m_refEntity, std::format("升级进度{0}/{1}", i + 1, MAX进度));
	}

	EntitySystem::BroadcastEntity描述(m_refEntity, "");
	auto& spacePlayer = m_refEntity.m_refSpace.GetSpacePlayer(m_refEntity);
	spacePlayer.升级单位属性完成(单位, 属性, m_refEntity);
	std::weak_ptr<PlayerGateSession_Game> GetPlayerGateSession(const std::string & refStrNickName);
	auto wp = GetPlayerGateSession(EntitySystem::GetNickName( m_refEntity));
	if (!wp.expired())
		wp.lock()->Send单位属性();

	co_return true;
}