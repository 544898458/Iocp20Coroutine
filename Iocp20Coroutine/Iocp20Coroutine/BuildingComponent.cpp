#include "pch.h"
#include "BuildingComponent.h"
#include "Entity.h"
#include "../CoRoutine/CoRpc.h"
#include "../CoRoutine/CoTimer.h"
#include "AiCo.h"
#include "PlayerComponent.h"
#include "PlayerGateSession_Game.h"
#include "��λ.h"
#include "AttackComponent.h"
#include "�ɼ�Component.h"


void BuildingComponent::AddComponent(Entity& refThis, PlayerGateSession_Game& refSession, const ������λ���� ����)
{
	refThis.m_spBuilding = std::make_shared<BuildingComponent, PlayerGateSession_Game&, const ������λ����&>(refSession, ����);
}

BuildingComponent::BuildingComponent(PlayerGateSession_Game& refSession, const ������λ���� &����) :m_����(����)
{
	//if (!m_coAddMoney.Finished())
	//{
	//	LOG(INFO) << "ǰһ���콨��Э�̻�û����";
	//	return;
	//}
	//m_coAddMoney = AiCo::AddMoney(refSession, m_cancelAddMoney);
	//m_coAddMoney.Run();
}

void BuildingComponent::TryCancel(Entity& refEntity)
{
	m_TaskCancel���.TryCancel();
}

void BuildingComponent::���(PlayerGateSession_Game& refGateSession, Entity& refEntity)
{
	CHECK_VOID(m_fun����λ);
	++m_i�ȴ������;
	m_TaskCancel���.TryRun(m_fun����λ(*this, refGateSession, refEntity));
}

CoTask<SpEntity> BuildingComponent::Co����λ(BuildingComponent& refThis, PlayerGateSession_Game& refGateSession, Entity& refEntity, const ���λ���� ����)
{
	��λ::���λ���� ����;
	if (!��λ::Find���λ����(����, ����))
	{
		co_return{};
	}
	while (0 < refThis.m_i�ȴ������)
	{
		--refThis.m_i�ȴ������;
		//�ȿ�Ǯ
		const auto& [stop, responce] = co_await AiCo::ChangeMoney(refGateSession, ����.����.u32����Ǯ, false, refThis.m_TaskCancel���.cancel);
		if (stop)
		{
			LOG(WARNING) << "Э��RPC���,error=" << responce.error << ",finalMoney=" << responce.finalMoney << ",rpcSn=" << responce.msg.rpcSnId;
			co_return{};
		}
		//��ʱ
		using namespace std;
		if (co_await CoTimer::Wait(1s, refThis.m_TaskCancel���.cancel))
		{
			co_return{};
		}

		LOG(INFO) << "Э��RPC����,error=" << responce.error << ",finalMoney=" << responce.finalMoney;
		CHECK_NOTNULL_CO_RET_0(refGateSession.m_pCurSpace);
		const auto posBuilding = refEntity.m_Pos;
		Position pos = { posBuilding.x - 2 + std::rand() % 4, posBuilding.z + 2 };
		auto spNewEntity = std::make_shared<Entity, const Position&, Space&, const std::string&, const std::string&>(
			pos, *refGateSession.m_pCurSpace, ����.����.strPrefabName, ����.����.strName);
		if (stop)
		{
			LOG(WARNING) << "��Ǯʧ��";
			co_return{};
		}
		spNewEntity->AddComponentPlayer(refGateSession);
		AttackComponent::AddComponent(*spNewEntity);
		refGateSession.m_vecSpEntity.insert(spNewEntity);//�Լ����Ƶĵ�λ
		refGateSession.m_pCurSpace->m_mapEntity.insert({ (int64_t)spNewEntity.get() ,spNewEntity });//ȫ��ͼ��λ

		spNewEntity->BroadcastEnter();
		co_return spNewEntity;
	}
}

CoTaskBool BuildingComponent::Co���(BuildingComponent& refThis, PlayerGateSession_Game& refGateSession, Entity& refEntity)
{
	auto spEntity = co_await Co����λ(refThis, refGateSession, refEntity, ��);
	if (!spEntity)
	{
		assert(false);
		co_return false;
	}
	co_return false;
}

CoTaskBool BuildingComponent::Co�칤�̳�(BuildingComponent& refThis, PlayerGateSession_Game& refGateSession, Entity& refEntity)
{
	auto spEntity = co_await Co����λ(refThis, refGateSession, refEntity, ���̳�);
	if (!spEntity)
	{
		assert(false);
		co_return false;
	}

	�ɼ�Component::AddComponent(refEntity);
	co_return false;
}
