#include "pch.h"
#include "造建筑Component.h"
#include "单位.h"
#include "AiCo.h"
#include "Entity.h"
#include "EntitySystem.h"
#include "PlayerComponent.h"
#include "AttackComponent.h"
#include "PlayerGateSession_Game.h"
#include "../CoRoutine/CoTimer.h"
#include "../IocpNetwork/StrConv.h"

void 造建筑Component::AddComponent(Entity& refEntity, PlayerGateSession_Game& refGateSession, const 活动单位类型 类型)
{
	refEntity.m_sp造建筑 = std::make_shared<造建筑Component, PlayerGateSession_Game&, Entity&, const 活动单位类型>(refGateSession, refEntity, std::forward<const 活动单位类型&&>(类型));
}

bool 造建筑Component::正在建造(Entity& refEntity)
{
	if (!refEntity.m_sp造建筑)
		return false;

	return refEntity.m_sp造建筑->m_cancel造建筑.operator bool();
}

造建筑Component::造建筑Component(PlayerGateSession_Game& refSession, Entity& refEntity, const 活动单位类型 类型) :m_refEntity(refEntity)
{
	switch (类型)
	{
	case 工程车:
		m_set可造类型.insert(基地);
		m_set可造类型.insert(民房);
		m_set可造类型.insert(兵厂);
		m_set可造类型.insert(地堡);
		break;
	default:
		break;
	}
}

CoTaskBool 造建筑Component::Co造建筑(const Position refPos, const 建筑单位类型 类型)
{
	if (m_set可造类型.end() == m_set可造类型.find(类型))
	{
		co_return false;
	}

	PlayerComponent::播放声音(m_refEntity, "TSCYes01");

	//先走到目标点
	if (co_await AiCo::WalkToPos(m_refEntity, refPos, m_cancel造建筑, 5))
		co_return true;

	//然后开始扣钱建造
	auto spEntity建筑 = co_await m_refEntity.m_spPlayer->m_refSession.CoAddBuilding(类型, refPos);
	if (!spEntity建筑)
		co_return false;

	if (co_await Co建造过程(spEntity建筑, m_cancel造建筑))
		co_return true;

	co_return false;
}

CoTaskBool 造建筑Component::Co建造过程(WpEntity wpEntity建筑, FunCancel& cancel)
{
	PlayerComponent::播放声音(m_refEntity, "TSCTra00");

	KeepCancel kc(cancel);
	std::weak_ptr<BuildingComponent> wpBuilding(wpEntity建筑.lock()->m_spBuilding);

	while (!wpBuilding.expired() && MAX建造百分比 > wpBuilding.lock()->m_n建造进度百分比)
	{
		if (co_await CoTimer::WaitNextUpdate(cancel))
			co_return true;
		if (wpBuilding.expired())
			co_return true;

		BuildingComponent& refBuilding = *wpBuilding.lock();
		++refBuilding.m_n建造进度百分比;

		std::ostringstream oss;
		if (MAX建造百分比 <= refBuilding.m_n建造进度百分比)
			oss << "建造完成";
		else
			oss << "正在建造:" << refBuilding.m_n建造进度百分比 << "%";

		assert(!wpEntity建筑.expired());
		EntitySystem::BroadcastEntity描述(*wpEntity建筑.lock(), oss.str());
		if (m_refEntity.m_spPlayer)
			m_refEntity.m_spPlayer->m_refSession.Send资源();
	}

	if (m_refEntity.m_spAttack)
	{
		switch (m_refEntity.m_spAttack->m_类型)
		{
		case 工程车:PlayerComponent::播放声音(m_refEntity, "TSCUpd00"); break;
		default:break;
		}
	}
	co_return 0;
}

void 造建筑Component::TryCancel()
{
	if (m_cancel造建筑)
	{
		//LOG(INFO) << "调用m_cancel";
		m_cancel造建筑();
	}
	else
	{

	}
}