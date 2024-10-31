#include "pch.h"
#include "����λComponent.h"
#include "PlayerGateSession_Game.h"
#include "�ɼ�Component.h"
#include "��λ.h"
#include "../CoRoutine/CoRpc.h"
#include "../CoRoutine/CoTimer.h"
#include "AiCo.h"
#include "AttackComponent.h"
#include "DefenceComponent.h"
#include "PlayerComponent.h"
#include "��Component.h"

void ����λComponent::AddComponet(Entity &refEntity,PlayerGateSession_Game &refGateSession, const ������λ���� ����)
{
	refEntity.m_sp����λ = std::make_shared<����λComponent, PlayerGateSession_Game&, Entity&, const ������λ���� >(refGateSession, refEntity, std::forward<const ������λ����&&>(����));
}

����λComponent::����λComponent(PlayerGateSession_Game& refSession, Entity& refEntity, const ������λ���� ����):m_refEntity(refEntity)
{
	switch (����)
	{
	case ����:m_set��������.insert(���̳�); break;
	case ����:
		m_set��������.insert(��);
		m_set��������.insert(��ս��);
		break;
	default:
		break;
	}
}

void ����λComponent::���(PlayerGateSession_Game& refGateSession, Entity& refEntity, const ���λ���� ����)
{
	//CHECK_VOID(m_fun����λ);
	if (refGateSession.���λ������������е�() >= refGateSession.���λ����())
	{
		refGateSession.Say("�񷿲���"); //Additional supply depots required.��Ҫ�����ʳ��
		return;
	}
	if (m_set��������.end() == m_set��������.find(����))
	{
		refGateSession.Say("�첻�����ֵ�λ");
		return;
	}

	m_list�ȴ���.emplace_back(����);//++m_i�ȴ������;
	m_TaskCancel����λ.TryRun(Co����λ(refGateSession));
}


void ����λComponent::TryCancel(Entity& refEntity)
{
	m_TaskCancel����λ.TryCancel();
}

CoTaskBool ����λComponent::Co����λ(PlayerGateSession_Game& refGateSession)
{
	while (!m_list�ȴ���.empty())
	{
		const auto ����(m_list�ȴ���.front());
		��λ::���λ���� ����;
		if (!��λ::Find���λ����(����, ����))
		{
			co_return{};
		}
		using namespace std;
		const auto posBuilding = m_refEntity.m_Pos;
		Position pos = { posBuilding.x + std::rand() % 10, posBuilding.z + 3 };
		bool CrowdTool��վ��(const Position & refPos);
		if (!CrowdTool��վ��(pos))
		{
			refGateSession.Say("�˴����ɷ���");
			if (co_await CoTimer::Wait(1s, m_TaskCancel����λ.cancel))
			{
				co_return{};
			}
			continue;
		}

		//�ȿ�Ǯ
		const auto& [stop, responce] = co_await AiCo::ChangeMoney(refGateSession, ����.����.u16���ľ����, false, m_TaskCancel����λ.cancel);
		if (stop)
		{
			LOG(WARNING) << "Э��RPC���,error=" << responce.error << ",finalMoney=" << responce.finalMoney << ",rpcSn=" << responce.msg.rpcSnId;
			co_return{};
		}
		//��ʱ
		if (co_await CoTimer::Wait(1s, m_TaskCancel����λ.cancel))
		{
			co_return{};
		}

		LOG(INFO) << "Э��RPC����,error=" << responce.error << ",finalMoney=" << responce.finalMoney;
		CHECK_CO_RET_0(!refGateSession.m_wpSpace.expired());
		auto sp = refGateSession.m_wpSpace.lock();
		auto spNewEntity = std::make_shared<Entity, const Position&, Space&, const std::string&, const std::string&>(
			pos, *sp, ����.����.strPrefabName, ����.����.strName);
		spNewEntity->m_f������� = ����.f�������;
		PlayerComponent::AddComponent(*spNewEntity,refGateSession);
		AttackComponent::AddComponent(*spNewEntity, ����);
		DefenceComponent::AddComponent(*spNewEntity, ����.����.u16��ʼHp);
		��Component::AddComponent(*spNewEntity);
		refGateSession.m_setSpEntity.insert(spNewEntity);//�Լ����Ƶĵ�λ
		sp->m_mapEntity.insert({ (int64_t)spNewEntity.get() ,spNewEntity });//ȫ��ͼ��λ

		switch (����)
		{
		case ���̳�:
			�ɼ�Component::AddComponent(*spNewEntity);
			refGateSession.Say("���̳����Կ�����!");//SCV, good to go, sir. SCV���Կ�����
			break;
		case ��:
			refGateSession.Say("��˵����Ҫ���ҵĹ�ͷ��");//You want a piece of me, boy?��Ҫ�ҵ�һ������С�ӣ�
			break;
		case ��ս��:
			refGateSession.Say("��˵����Ҫ�ҵĸ�ͷ��");//You want a piece of me, boy?��Ҫ�ҵ�һ������С�ӣ�
			break;

		default:break;
		}

		if (m_list�ȴ���.empty())
		{
			LOG(ERROR) << "err";
			assert(false);
			co_return{};
		}

		m_list�ȴ���.pop_front();//--refThis.m_i�ȴ������;

		spNewEntity->BroadcastEnter();
		refGateSession.Send��Դ();
	}
}

uint16_t ����λComponent::�ȴ���Count()const
{
	return (uint16_t)m_list�ȴ���.size(); 
}