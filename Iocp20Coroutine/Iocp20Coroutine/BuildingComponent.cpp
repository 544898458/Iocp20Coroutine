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
	if (!m_coAddMoney.Finished())
	{
		LOG(INFO) << "ǰһ���콨��Э�̻�û����";
		return;
	}
	m_coAddMoney = AiCo::AddMoney(refSession, m_cancelAddMoney);
	m_coAddMoney.Run();
}

void BuildingComponent::TryCancel(Entity& refEntity)
{
	if (m_cancelAddMoney)
	{
		//LOG(INFO) << "����m_cancel";
		m_cancelAddMoney();
		assert(m_coAddMoney.Finished());
	}
	else
	{
		//LOG(INFO) << "m_cancel�ǿյģ�û��Ҫȡ����Э��";
		if (!m_coAddMoney.Finished())
		{
			LOG(ERROR) << "Э��û������ȴ��ǰ�����m_cancel";
			assert(false);
		}
	}

	assert(m_coAddMoney.Finished());
}

void BuildingComponent::���(PlayerGateSession_Game& refGateSession, Entity& refEntity)
{
	++m_i�ȴ������;
	if (m_co���.Finished())
	{
		m_co��� = Co���(refGateSession, refEntity);
		m_co���.Run();
	}
}

CoTask<int> BuildingComponent::Co���(PlayerGateSession_Game& refGateSession, Entity& refEntity)
{
	while (0 < m_i�ȴ������)
	{
		--m_i�ȴ������;
		//�ȿ�Ǯ
		const auto& [stop, responce] = co_await AiCo::ChangeMoney(refGateSession, 5, false, m_cancel���);
		if (stop)
		{
			LOG(WARNING) << "Э��RPC���,error=" << responce.error << ",finalMoney=" << responce.finalMoney << ",rpcSn=" << responce.msg.rpcSnId;
			co_return 0;
		}
		//��ʱ
		using namespace std;
		if (co_await CoTimer::Wait(1s, m_cancel���))
		{
			co_return 0;
		}

		LOG(INFO) << "Э��RPC����,error=" << responce.error << ",finalMoney=" << responce.finalMoney;
		CHECK_NOTNULL_CO_RET_0(refGateSession.m_pCurSpace);
		const auto posBuilding = refEntity.m_Pos;
		Position pos = { posBuilding.x - 2 + std::rand() % 4, posBuilding.z + 2 };
		auto spNewEntity = std::make_shared<Entity, const Position&, Space&, const std::string&, const std::string&>(
			pos, *refGateSession.m_pCurSpace, "altman-blue", "��ս��");
		if (stop)
		{
			LOG(WARNING) << "��Ǯʧ��";
			co_return 0;
		}
		spNewEntity->AddComponentPlayer(refGateSession);
		spNewEntity->AddComponentAttack();
		refGateSession.m_vecSpEntity.insert(spNewEntity);//�Լ����Ƶĵ�λ
		refGateSession.m_pCurSpace->m_mapEntity.insert({ (int64_t)spNewEntity.get() ,spNewEntity });//ȫ��ͼ��λ

		spNewEntity->BroadcastEnter();
	}
}
