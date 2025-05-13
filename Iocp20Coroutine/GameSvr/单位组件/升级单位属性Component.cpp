#pragma once
#include "pch.h"
#include "升级单位属性Component.h"
#include "造活动单位Component.h"
#include "BuildingComponent.h"
#include "解锁单位Component.h"
#include "../Entity.h"
#include "../../CoRoutine/CoTimer.h"
#include "../Space.h"
#include "../EntitySystem.h"
#include "../PlayerGateSession_Game.h"
#include "../枚举/属性类型.h"

升级单位属性Component::升级单位属性Component(Entity& ref) :m_refEntity(ref)
{
	switch (m_refEntity.m_类型)
	{
	case 兵营:
		m_map可升级单位属性 = { {枪兵, {攻击}}, {近战兵, {防御}} };
		break;
	case 虫营:
		m_map可升级单位属性 = { {枪虫, {防御}}, {近战虫, {攻击}} };
		break;
	case 重车厂:
		m_map可升级单位属性 = { {三色坦克, {移动速度}} };
		break;
	case 机场:
		m_map可升级单位属性 = { {飞机, {攻击前摇_伤害耗时}} };
		break;
	case 拟态源:
		m_map可升级单位属性 = { {绿色坦克, {攻击前摇_伤害耗时}} };
		break;
	case 飞塔:
		m_map可升级单位属性 = { {飞虫, {移动速度}} };
		break;
	default:
		LOG(ERROR) << "不能升级单位属性:" << m_refEntity.m_类型;
		break;
	}
}

void 升级单位属性Component::AddComponent(Entity& refEntity)
{
	refEntity.m_up升级单位属性.reset(new 升级单位属性Component(refEntity));
}

void 升级单位属性Component::OnEntityDestroy(const bool bDestroy)
{
	m_cancel升级单位属性.TryCancel();
}


void 升级单位属性Component::升级(const 单位类型 单位, const 属性类型 属性)
{
	CHECK_RET_VOID(m_refEntity.m_upBuilding);
	if (!m_refEntity.m_upBuilding->已造好())
	{
		PlayerGateSession_Game::播放声音Buzz(m_refEntity, "还没造好建筑，不能升级单位属性");
		return;
	}
	if (正在升级())
	{
		PlayerGateSession_Game::播放声音Buzz(m_refEntity, "正在升级");
		return;
	}
	if (m_refEntity.m_up解锁单位 && m_refEntity.m_up解锁单位->正在解锁())
	{
		PlayerGateSession_Game::播放声音Buzz(m_refEntity, "正在解锁单位，不能升级属性");
		return;
	}

	{
		//判断可升级单位
		auto iterFind单位 = m_map可升级单位属性.find(单位);
		if (iterFind单位 == m_map可升级单位属性.end())
		{
			PlayerGateSession_Game::播放声音Buzz(m_refEntity, "不能升级此单位的属性");
			return;
		}

		//判断可升级此属性
		auto iterFind属性 = iterFind单位->second.find(属性);
		if (iterFind单位->second.end() == iterFind属性)
		{
			PlayerGateSession_Game::播放声音Buzz(m_refEntity, "不能升级此单位此属性");
			return;
		}
	}

	auto& spacePlayer = m_refEntity.m_refSpace.GetSpacePlayer(m_refEntity);

	if (!spacePlayer.开始升级单位属性(单位, 属性, m_refEntity))
		return;

	Co升级(单位, 属性).RunNew();
}

bool 升级单位属性Component::正在升级() const
{
	return m_cancel升级单位属性.operator bool();
}

CoTaskBool 升级单位属性Component::Co升级(const 单位类型 单位, const 属性类型 属性)
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
	auto wp = GetPlayerGateSession(EntitySystem::GetNickName(m_refEntity));
	if (!wp.expired())
		wp.lock()->Send单位属性();

	co_return true;
}