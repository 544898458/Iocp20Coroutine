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
#include "BuildingComponent.h"

void ����λComponent::AddComponent(Entity& refEntity, const ��λ���� ����)
{
	refEntity.m_sp����λ = std::make_shared<����λComponent, Entity&, const ��λ���� >(refEntity, std::forward<const ��λ����&&>(����));
}

����λComponent::����λComponent(Entity& refEntity, const ��λ���� ����) :m_refEntity(refEntity)
{
	switch (����)
	{
	case ����:m_set��������.insert(���̳�); break;
	case ����:
		m_set��������.insert(��);
		m_set��������.insert(��ս��);
		m_set��������.insert(��ɫ̹��);
		break;
	default:
		break;
	}
}

bool ����λComponent::����(const ��λ���� ����)const
{
	return m_set��������.end() != m_set��������.find(����);
}

void ����λComponent::���(PlayerGateSession_Game& refGateSession, const ��λ���� ����)
{
	//CHECK_VOID(m_fun����λ);
	if (refGateSession.���λ������������е�() >= refGateSession.���λ����())
	{
		//refGateSession.��������("tadErr02", "�񷿲���"); //Additional supply depots required.��Ҫ�����ʳ��
		refGateSession.��������("����/�񷿲���Ů��������", "�񷿲���");
		return;
	}
	if (!����(����))
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
	m_TaskCancel����λ.TryRun(Co����λ());
}


void ����λComponent::TryCancel(Entity& refEntity)
{
	m_TaskCancel����λ.TryCancel();
}

CoTaskBool ����λComponent::Co����λ()
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
		auto &refSpace = m_refEntity.m_refSpace;
		using namespace std;
		const auto posBuilding = m_refEntity.Pos();
		Position pos = { posBuilding.x + std::rand() % 10, posBuilding.z + +std::rand() % 10 };
		{
			const auto ok = refSpace.CrowdToolFindNerestPos(pos);
			_ASSERTok);
		}
		
		if (!refSpace.CrowdTool��վ��(pos))
		{
			PlayerComponent::��������(m_refEntity, "�˴����ɷ���");
			if (co_await CoTimer::Wait(1s, m_TaskCancel����λ.cancel))
			{
				m_list�ȴ���.clear();
				co_return{};
			}
			continue;
		}

		if (����.����.u16����ȼ���� > Space::GetSpacePlayer(m_refEntity).m_u32ȼ����)
		{
			//std::ostringstream oss;
			PlayerComponent::��������(m_refEntity, "����/ȼ������������", "ȼ������");//(low error beep) Insufficient Vespene Gas.������ 
			//Sayϵͳ(oss.str());
			m_list�ȴ���.clear();
			co_return{};
		}
		refSpace.GetSpacePlayer(m_refEntity).m_u32ȼ���� -= ����.����.u16����ȼ����;

		//�ȿ�Ǯ
		//const auto& [stop, responce] = co_await AiCo::ChangeMoney(refGateSession, ����.����.u16���ľ����, false, m_TaskCancel����λ.cancel);
		//if (stop)
		//{
		//	LOG(WARNING) << "Э��RPC���,error=" << responce.error << ",finalMoney=" << responce.finalMoney << ",rpcSn=" << responce.msg.rpcSnId;
		//	m_list�ȴ���.clear();
		//	co_return{};
		//}
		//if (0 != responce.error)
		if (����.����.u16���ľ���� > Space::GetSpacePlayer(m_refEntity).m_u32�����)
		{
			//refGateSession.m_u32ȼ���� += ����.����.u16����ȼ����;
			PlayerComponent::��������(m_refEntity, "����/�������������", "��������");//Sayϵͳ("��������" + ����.����.u16���ľ����);
			m_list�ȴ���.clear();
			co_return{};
		}
		Space::GetSpacePlayer(m_refEntity).m_u32�����-= ����.����.u16���ľ����;
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

		//LOG(INFO) << "Э��RPC����,error=" << responce.error << ",finalMoney=" << responce.finalMoney;
		//CHECK_CO_RET_0(!refGateSession.m_wpSpace.expired());
		SpEntity spNewEntity = m_refEntity.m_refSpace.����λ(m_refEntity.m_spPlayer, EntitySystem::GetNickName(m_refEntity), pos, ����, ����);

		//if (m_list�ȴ���.empty())
		//{
		//	LOG(ERROR) << "err";
		//	_ASSERTfalse);
		//	co_return{};
		//}
	}

	EntitySystem::BroadcastEntity����(m_refEntity, "������");
}

uint16_t ����λComponent::�ȴ���Count()const
{
	return (uint16_t)m_list�ȴ���.size();
}