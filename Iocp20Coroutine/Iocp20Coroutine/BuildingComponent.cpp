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
#include "DefenceComponent.h"
#include "�ɼ�Component.h"
#include "��ʱ�赲Component.h"

void BuildingComponent::AddComponent(Entity& refThis, PlayerGateSession_Game& refSession, const ������λ���� ����, float f��߳�)
{
	refThis.m_spBuilding = std::make_shared<BuildingComponent, PlayerGateSession_Game&, const ������λ����&>(refSession, ����);
	��ʱ�赲Component::AddComponent(refThis, f��߳�);
}

BuildingComponent::BuildingComponent(PlayerGateSession_Game& refSession, const ������λ����& ����) :m_����(����)
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
	if (refGateSession.���λ������������е�() >= refGateSession.���λ����())
	{
		refGateSession.Say("�񷿲���"); //Additional supply depots required.��Ҫ�����ʳ��
		return;
	}
	++m_i�ȴ������;
	m_TaskCancel���.TryRun(m_fun����λ(*this, refGateSession, refEntity));
}

CoTaskBool BuildingComponent::Co����λ(BuildingComponent& refThis, PlayerGateSession_Game& refGateSession, Entity& refEntity, const ���λ���� ����, std::function<void(Entity&)> fun)
{
	��λ::���λ���� ����;
	if (!��λ::Find���λ����(����, ����))
	{
		co_return{};
	}
	while (0 < refThis.m_i�ȴ������)
	{
		//�ȿ�Ǯ
		const auto& [stop, responce] = co_await AiCo::ChangeMoney(refGateSession, ����.����.u32���ľ����, false, refThis.m_TaskCancel���.cancel);
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
		Position pos = { posBuilding.x + std::rand() % 10, posBuilding.z + 3 };
		bool CrowdToolfindNearest(Position & refPos);
		CrowdToolfindNearest(pos);
		auto spNewEntity = std::make_shared<Entity, const Position&, Space&, const std::string&, const std::string&>(
			pos, *refGateSession.m_pCurSpace, ����.����.strPrefabName, ����.����.strName);
		spNewEntity->m_f������� = ����.f�������;
		spNewEntity->AddComponentPlayer(refGateSession);
		AttackComponent::AddComponent(*spNewEntity);
		DefenceComponent::AddComponent(*spNewEntity);
		refGateSession.m_setSpEntity.insert(spNewEntity);//�Լ����Ƶĵ�λ
		refGateSession.m_pCurSpace->m_mapEntity.insert({ (int64_t)spNewEntity.get() ,spNewEntity });//ȫ��ͼ��λ
		if (fun)
			fun(*spNewEntity);

		--refThis.m_i�ȴ������;

		spNewEntity->BroadcastEnter();
		refGateSession.Send��Դ();
	}
}

CoTaskBool BuildingComponent::Co���(BuildingComponent& refThis, PlayerGateSession_Game& refGateSession, Entity& refEntity)
{
	return Co����λ(refThis, refGateSession, refEntity, ��);
}

CoTaskBool BuildingComponent::Co�칤�̳�(BuildingComponent& refThis, PlayerGateSession_Game& refGateSession, Entity& refEntity)
{
	return Co����λ(refThis, refGateSession, refEntity, ���̳�, [](Entity& refEntity) {�ɼ�Component::AddComponent(refEntity); });
}
