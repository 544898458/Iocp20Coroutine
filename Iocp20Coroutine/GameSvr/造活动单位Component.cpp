#include "pch.h"
#include "����λComponent.h"
#include "PlayerGateSession_Game.h"
#include "�ɼ�Component.h"
#include "�콨��Component.h"
#include "��λ.h"
#include "../CoRoutine/CoRpc.h"
#include "../CoRoutine/CoTimer.h"
#include "AiCo.h"
#include "AttackComponent.h"
#include "DefenceComponent.h"
#include "PlayerComponent.h"
#include "��Component.h"
#include "EntitySystem.h"

void ����λComponent::AddComponent(Entity& refEntity, PlayerGateSession_Game& refGateSession, const ������λ���� ����)
{
	refEntity.m_sp����λ = std::make_shared<����λComponent, PlayerGateSession_Game&, Entity&, const ������λ���� >(refGateSession, refEntity, std::forward<const ������λ����&&>(����));
}

����λComponent::����λComponent(PlayerGateSession_Game& refSession, Entity& refEntity, const ������λ���� ����) :m_refEntity(refEntity)
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
		refGateSession.��������("tadErr02", "�񷿲���"); //Additional supply depots required.��Ҫ�����ʳ��
		return;
	}
	if (m_set��������.end() == m_set��������.find(����))
	{
		refGateSession.Sayϵͳ("�첻�����ֵ�λ");
		return;
	}

	if (!m_refEntity.m_spBuilding->�����())
	{
		refGateSession.Sayϵͳ("������û���");
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
		m_list�ȴ���.pop_front();//--refThis.m_i�ȴ������;
		��λ::���λ���� ����;
		if (!��λ::Find���λ����(����, ����))
		{
			co_return{};
		}
		using namespace std;
		const auto posBuilding = m_refEntity.m_Pos;
		Position pos = { posBuilding.x + std::rand() % 10, posBuilding.z + +std::rand() % 10 };
		{
			const auto ok = m_refEntity.m_refSpace.CrowdToolFindNerestPos(pos);
			assert(ok);
		}
		bool CrowdTool��վ��(CrowdToolState & refCrowTool, const Position & refPos);
		CHECK_CO_RET_FALSE(!refGateSession.m_wpSpace.expired());
		auto& refCrowdToolState = *refGateSession.m_wpSpace.lock()->m_spCrowdToolState;

		if (!CrowdTool��վ��(refCrowdToolState, pos))
		{
			refGateSession.��������("�˴����ɷ���");
			if (co_await CoTimer::Wait(1s, m_TaskCancel����λ.cancel))
			{
				m_list�ȴ���.clear();
				co_return{};
			}
			continue;
		}

		if (����.����.u16����ȼ���� > refGateSession.m_u32ȼ����)
		{
			//std::ostringstream oss;
			refGateSession.��������("����/ȼ������������", "ȼ������");//(low error beep) Insufficient Vespene Gas.������ 
			//Sayϵͳ(oss.str());
			m_list�ȴ���.clear();
			co_return{};
		}
		refGateSession.m_u32ȼ���� -= ����.����.u16����ȼ����;

		//�ȿ�Ǯ
		const auto& [stop, responce] = co_await AiCo::ChangeMoney(refGateSession, ����.����.u16���ľ����, false, m_TaskCancel����λ.cancel);
		if (stop)
		{
			LOG(WARNING) << "Э��RPC���,error=" << responce.error << ",finalMoney=" << responce.finalMoney << ",rpcSn=" << responce.msg.rpcSnId;
			m_list�ȴ���.clear();
			co_return{};
		}
		if (0 != responce.error)
		{
			refGateSession.m_u32ȼ���� += ����.����.u16����ȼ����;
			refGateSession.��������("����/�������������", "��������");//Sayϵͳ("��������" + ����.����.u16���ľ����);
			m_list�ȴ���.clear();
			co_return{};
		}
		//��ʱ
		//if (co_await CoTimer::Wait(1s, m_TaskCancel����λ.cancel))
		//{
		//	co_return{};
		//}
		const int MAX���� = 10;
		for (int i = 0; i < 10; ++i)
		{
			if (co_await CoTimer::Wait(300ms, m_TaskCancel����λ.cancel))
			{
				m_list�ȴ���.clear();
				co_return{};
			}
			EntitySystem::BroadcastEntity����(m_refEntity, std::format("�������{0},��ǰ��λ����{1}/{2}", m_list�ȴ���.size(), i, MAX����));
		}

		LOG(INFO) << "Э��RPC����,error=" << responce.error << ",finalMoney=" << responce.finalMoney;
		CHECK_CO_RET_0(!refGateSession.m_wpSpace.expired());
		SpEntity spNewEntity = ����λ(refGateSession, pos, ����, ����);

		//if (m_list�ȴ���.empty())
		//{
		//	LOG(ERROR) << "err";
		//	assert(false);
		//	co_return{};
		//}
	}

	EntitySystem::BroadcastEntity����(m_refEntity, "������");
}

SpEntity ����λComponent::����λ(PlayerGateSession_Game& refGateSession, const Position& pos, const ��λ::���λ����& ����, const ���λ���� ����)
{
	auto sp = refGateSession.m_wpSpace.lock();
	SpEntity spNewEntity = std::make_shared<Entity, const Position&, Space&, const std::string&, const std::string&>(
		pos, *sp, ����.����.strPrefabName, ����.����.strName);
	spNewEntity->m_f������� = ����.ս��.f�������;
	PlayerComponent::AddComponent(*spNewEntity, refGateSession);
	AttackComponent::AddComponent(*spNewEntity, ����, ����.ս��.f��������, ����.ս��.f�˺�);
	DefenceComponent::AddComponent(*spNewEntity, ����.����.u16��ʼHp);
	��Component::AddComponent(*spNewEntity);
	refGateSession.m_setSpEntity.insert(spNewEntity);//�Լ����Ƶĵ�λ
	sp->m_mapEntity.insert({ (int64_t)spNewEntity.get() ,spNewEntity });//ȫ��ͼ��λ

	switch (����)
	{
	case ���̳�:
		�ɼ�Component::AddComponent(*spNewEntity);
		�콨��Component::AddComponent(*spNewEntity, refGateSession, ����);
		refGateSession.��������("TSCRdy00");//refGateSession.Say������ʾ("���̳����Կ�����!");//SCV, good to go, sir. SCV���Կ�����
		break;
	case ��:
		refGateSession.��������("TMaRdy00");//refGateSession.Say������ʾ("��˵����Ҫ���ҵĹ�ͷ��");//You want a piece of me, boy?��Ҫ�ҵ�һ������С�ӣ�
		break;
	case ��ս��:
		refGateSession.��������("tfbRdy00");//refGateSession.Say������ʾ("��˵����Ҫ�ҵĸ�ͷ��");//You want a piece of me, boy?��Ҫ�ҵ�һ������С�ӣ�
		break;

	default:break;
	}

	spNewEntity->BroadcastEnter();
	refGateSession.Send��Դ();

	return spNewEntity;
}

uint16_t ����λComponent::�ȴ���Count()const
{
	return (uint16_t)m_list�ȴ���.size();
}