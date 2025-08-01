#pragma once
#include "pch.h"
#include "解锁单位Component.h"
#include "造活动单位Component.h"
#include "BuildingComponent.h"
#include "升级单位属性Component.h"
#include "PlayerComponent.h"
#include "../Entity.h"
#include "../../CoRoutine/CoTimer.h"
#include "../Space.h"
#include "../EntitySystem.h"
#include "../PlayerGateSession_Game.h"

解锁单位Component::解锁单位Component(Entity& ref) :m_refEntity(ref), m_cancel解锁单位("m_cancel解锁单位")
{
}

void 解锁单位Component::AddComponent(Entity& refEntity)
{
	refEntity.AddComponentOnDestroy(&Entity::m_up解锁单位, refEntity);
}

void 解锁单位Component::OnEntityDestroy(const bool bDestroy)
{
	m_cancel解锁单位.TryCancel();
}


void 解锁单位Component::解锁单位(const 单位类型 类型)
{
	CHECK_RET_VOID(m_refEntity.m_upBuilding);
	if (!m_refEntity.m_upBuilding->已造好())
	{
		PlayerGateSession_Game::播放声音Buzz(m_refEntity, "还没造好建筑，不能解锁单位");
		return;
	}

	if (正在解锁())
	{
		PlayerGateSession_Game::播放声音Buzz(m_refEntity, "正在解锁");
		return;
	}

	if (m_refEntity.m_up升级单位属性 && m_refEntity.m_up升级单位属性->正在升级())
	{
		PlayerGateSession_Game::播放声音Buzz(m_refEntity, "正在升级，不能解锁单位");
		return;
	}


	auto& spacePlayer = m_refEntity.m_refSpace.GetSpacePlayer(m_refEntity);
	if (!spacePlayer.开始解锁单位(类型, m_refEntity))
		return;

	Co解锁单位(类型).RunNew();
}

bool 解锁单位Component::正在解锁() const
{
	return m_cancel解锁单位.operator bool();
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
	std::weak_ptr<PlayerGateSession_Game> GetPlayerGateSession(const std::string & refStrNickName);
	auto wp = GetPlayerGateSession(EntitySystem::GetNickName(m_refEntity));
	if (!wp.expired()){
		wp.lock()->Send已解锁单位();
		PlayerComponent::播放声音(m_refEntity, "语音/单位解锁完成女声正经版", "已解锁单位");
	}

	co_return true;
}