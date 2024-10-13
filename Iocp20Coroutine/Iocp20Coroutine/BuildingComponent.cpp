#include "pch.h"
#include "BuildingComponent.h"
#include "Entity.h"
#include "../CoRoutine/CoRpc.h"
#include "../CoRoutine/CoTimer.h"
#include "AiCo.h"
#include "PlayerComponent.h"
#include "PlayerGateSession_Game.h"

void Entity::AddComponentBuilding(PlayerGateSession_Game& refSession)
{
	m_spBuilding = std::make_shared<BuildingComponent, PlayerGateSession_Game&>(refSession);
}

BuildingComponent::BuildingComponent(PlayerGateSession_Game& refSession)
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
	++m_i等待造兵数;
	m_TaskCancel造兵.TryRun(Co造兵(refGateSession, refEntity));
}

CoTaskUint8 BuildingComponent::Co造兵(PlayerGateSession_Game& refGateSession, Entity& refEntity)
{
	while (0 < m_i等待造兵数)
	{
		--m_i等待造兵数;
		//先扣钱
		const auto& [stop, responce] = co_await AiCo::ChangeMoney(refGateSession, 5, false, m_TaskCancel造兵.cancel);
		if (stop)
		{
			LOG(WARNING) << "协程RPC打断,error=" << responce.error << ",finalMoney=" << responce.finalMoney << ",rpcSn=" << responce.msg.rpcSnId;
			co_return 0;
		}
		//耗时
		using namespace std;
		if (co_await CoTimer::Wait(1s, m_TaskCancel造兵.cancel))
		{
			co_return 0;
		}

		LOG(INFO) << "协程RPC返回,error=" << responce.error << ",finalMoney=" << responce.finalMoney;
		CHECK_NOTNULL_CO_RET_0(refGateSession.m_pCurSpace);
		const auto posBuilding = refEntity.m_Pos;
		Position pos = { posBuilding.x - 2 + std::rand() % 4, posBuilding.z + 2 };
		auto spNewEntity = std::make_shared<Entity, const Position&, Space&, const std::string&, const std::string&>(
			pos, *refGateSession.m_pCurSpace, "altman-blue", "近战兵");
		if (stop)
		{
			LOG(WARNING) << "扣钱失败";
			co_return 0;
		}
		spNewEntity->AddComponentPlayer(refGateSession);
		spNewEntity->AddComponentAttack();
		refGateSession.m_vecSpEntity.insert(spNewEntity);//自己控制的单位
		refGateSession.m_pCurSpace->m_mapEntity.insert({ (int64_t)spNewEntity.get() ,spNewEntity });//全地图单位

		spNewEntity->BroadcastEnter();
	}
}
