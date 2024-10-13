#include "pch.h"
#include "BuildingComponent.h"
#include "Entity.h"
#include "../CoRoutine/CoRpc.h"
#include "../CoRoutine/CoTimer.h"
#include "AiCo.h"
#include "PlayerComponent.h"
#include "PlayerGateSession_Game.h"
#include "单位.h"
#include "AttackComponent.h"
#include "采集Component.h"


void BuildingComponent::AddComponent(Entity& refThis, PlayerGateSession_Game& refSession, const 建筑单位类型 类型)
{
	refThis.m_spBuilding = std::make_shared<BuildingComponent, PlayerGateSession_Game&, const 建筑单位类型&>(refSession, 类型);
}

BuildingComponent::BuildingComponent(PlayerGateSession_Game& refSession, const 建筑单位类型 &类型) :m_类型(类型)
{
	//if (!m_coAddMoney.Finished())
	//{
	//	LOG(INFO) << "前一个造建筑协程还没返回";
	//	return;
	//}
	//m_coAddMoney = AiCo::AddMoney(refSession, m_cancelAddMoney);
	//m_coAddMoney.Run();
}

void BuildingComponent::TryCancel(Entity& refEntity)
{
	m_TaskCancel造兵.TryCancel();
}

void BuildingComponent::造兵(PlayerGateSession_Game& refGateSession, Entity& refEntity)
{
	CHECK_VOID(m_fun造活动单位);
	++m_i等待造兵数;
	m_TaskCancel造兵.TryRun(m_fun造活动单位(*this, refGateSession, refEntity));
}

CoTask<SpEntity> BuildingComponent::Co造活动单位(BuildingComponent& refThis, PlayerGateSession_Game& refGateSession, Entity& refEntity, const 活动单位类型 类型)
{
	单位::活动单位配置 配置;
	if (!单位::Find活动单位配置(类型, 配置))
	{
		co_return{};
	}
	while (0 < refThis.m_i等待造兵数)
	{
		--refThis.m_i等待造兵数;
		//先扣钱
		const auto& [stop, responce] = co_await AiCo::ChangeMoney(refGateSession, 配置.配置.u32消耗钱, false, refThis.m_TaskCancel造兵.cancel);
		if (stop)
		{
			LOG(WARNING) << "协程RPC打断,error=" << responce.error << ",finalMoney=" << responce.finalMoney << ",rpcSn=" << responce.msg.rpcSnId;
			co_return{};
		}
		//耗时
		using namespace std;
		if (co_await CoTimer::Wait(1s, refThis.m_TaskCancel造兵.cancel))
		{
			co_return{};
		}

		LOG(INFO) << "协程RPC返回,error=" << responce.error << ",finalMoney=" << responce.finalMoney;
		CHECK_NOTNULL_CO_RET_0(refGateSession.m_pCurSpace);
		const auto posBuilding = refEntity.m_Pos;
		Position pos = { posBuilding.x - 2 + std::rand() % 4, posBuilding.z + 2 };
		auto spNewEntity = std::make_shared<Entity, const Position&, Space&, const std::string&, const std::string&>(
			pos, *refGateSession.m_pCurSpace, 配置.配置.strPrefabName, 配置.配置.strName);
		if (stop)
		{
			LOG(WARNING) << "扣钱失败";
			co_return{};
		}
		spNewEntity->AddComponentPlayer(refGateSession);
		AttackComponent::AddComponent(*spNewEntity);
		refGateSession.m_vecSpEntity.insert(spNewEntity);//自己控制的单位
		refGateSession.m_pCurSpace->m_mapEntity.insert({ (int64_t)spNewEntity.get() ,spNewEntity });//全地图单位

		spNewEntity->BroadcastEnter();
		co_return spNewEntity;
	}
}

CoTaskBool BuildingComponent::Co造兵(BuildingComponent& refThis, PlayerGateSession_Game& refGateSession, Entity& refEntity)
{
	auto spEntity = co_await Co造活动单位(refThis, refGateSession, refEntity, 兵);
	if (!spEntity)
	{
		assert(false);
		co_return false;
	}
	co_return false;
}

CoTaskBool BuildingComponent::Co造工程车(BuildingComponent& refThis, PlayerGateSession_Game& refGateSession, Entity& refEntity)
{
	auto spEntity = co_await Co造活动单位(refThis, refGateSession, refEntity, 工程车);
	if (!spEntity)
	{
		assert(false);
		co_return false;
	}

	采集Component::AddComponent(refEntity);
	co_return false;
}
