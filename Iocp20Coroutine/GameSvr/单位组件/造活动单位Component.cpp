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
#include "AoiComponent.h"

void ����λComponent::AddComponent(Entity& refEntity)
{
	refEntity.m_sp����λ = std::make_shared<����λComponent, Entity& >(refEntity);
}

����λComponent::����λComponent(Entity& refEntity) :m_refEntity(refEntity)
{
	switch (refEntity.m_����)
	{
	case ����:m_set�������� = { ���̳�, ҽ�Ʊ� }; break;
	case ����:m_set�������� = { �ɻ� }; break;
	case ��Ӫ:m_set�������� = { ǹ��,��ս�� }; break;
	case �س���:m_set�������� = { ��ɫ̹�� }; break;
	case �׳�:m_set�������� = { ����, ��ս��, ǹ��, ��ɫ̹��, �ɳ�, ���� }; break;
	default:
		LOG(ERROR) << "δ֪�Ľ��쵥λ����," << refEntity.m_����;
		_ASSERT(!"δ֪�Ľ��쵥λ����");
		break;
	}
	m_pos����� = refEntity.Pos();
}

bool ����λComponent::����(const ��λ���� ����)
{
	return m_set��������.end() != m_set��������.find(����) && !Is�׳������ɱ�();
}

bool ����λComponent::�жϲ���ʾȱ�ٽ���(PlayerGateSession_Game& refGateSession, const ��λ���� ��λ)
{
	if (0 < m_refEntity.m_refSpace.Get��ҵ�λ��(refGateSession.NickName(), ��λ))
		return true;

	��λ::��λ���� ����;
	CHECK_RET_DEFAULT(��λ::Find��λ����(��λ, ����));

	refGateSession.��������Buzz(std::format("ȱ�� {0}", ����.strName));
	return false;
}

void ����λComponent::���(PlayerGateSession_Game& refGateSession, const ��λ���� ����)
{
	//CHECK_VOID(m_fun����λ);
	if (���� != ���� && refGateSession.���λ������������е�() >= refGateSession.���λ����())
	{
		//refGateSession.��������("tadErr02", "�񷿲���"); //Additional supply depots required.��Ҫ�����ʳ��
		switch (����)
		{
		case ����:
		case �ɳ�:
		case ��ս��:
		case ǹ��:
		case ��ɫ̹��:
			refGateSession.��������("����/���治��_����", "���治��");
			break;
		default:
			refGateSession.��������("����/�񷿲���Ů��������", "�񷿲���");
			break;
		}

		return;
	}
	if (!����(����))
	{
		refGateSession.Sayϵͳ("�첻�����ֵ�λ");
		return;
	}

#define CHECK_ȱ�ٵ�λ_RET(��λ) if(!�жϲ���ʾȱ�ٽ���(refGateSession, ��λ))return;

	switch (����)
	{
	case ǹ��:
	case ��ս��:
		CHECK_ȱ�ٵ�λ_RET(��Ӫ);
		break;
	case �ɳ�:
		CHECK_ȱ�ٵ�λ_RET(����);
		break;
	case ��ɫ̹��:
		CHECK_ȱ�ٵ�λ_RET(��̬Դ);
		break;
	default:
		break;
	}

	auto& spacePlayer = m_refEntity.m_refSpace.GetSpacePlayer(refGateSession.NickName());
	if (!spacePlayer.�ѽ���(����))
	{
		switch (����)
		{
		case ��ս��:
			refGateSession.��������Buzz("�����ڱ�Ӫ�н���");
			break;
		case ǹ��:
			refGateSession.��������Buzz("�����ڳ�Ӫ�н���");
			break;
		default:
			LOG(ERROR) << "δ֪��δ������λ����," << ����;
			break;
		}

		return;
	}

	if (m_refEntity.m_spBuilding && !m_refEntity.m_spBuilding->�����())
	{
		refGateSession.Sayϵͳ("������û���");
		return;
	}

	if (Is�׳������ɱ�())
	{
		refGateSession.��������Buzz("�׳������ɱ�");
		return;
	}

	m_list�ȴ���.emplace_back(����);//++m_i�ȴ������;
	m_TaskCancel����λ.TryRun(Co����λ());
}

bool ����λComponent::Is�׳�()const
{
	return �׳� == m_refEntity.m_����;
}

bool ����λComponent::Is�׳������ɱ�()
{
	return !m_TaskCancel����λ.co.Finished() && Is�׳�();
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
		��λ::���λ���� �;
		��λ::��λ���� ��λ;
		��λ::�������� ����;
		CHECK_CO_RET_FALSE(��λ::Find���λ����(����, �));
		CHECK_CO_RET_FALSE(��λ::Find��λ����(����, ��λ));
		CHECK_CO_RET_FALSE(��λ::Find��������(����, ����));
		auto& refSpace = m_refEntity.m_refSpace;
		using namespace std;
		const auto posBuilding = m_refEntity.Pos();
		Position pos = { posBuilding.x - 5 + std::rand() % 10, posBuilding.z - 5 + std::rand() % 10 };
		if (!Is�׳�())
		{
			const auto ok = refSpace.CrowdToolFindNerestPos(pos);
			_ASSERT(ok);
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
			PlayerComponent::��������(m_refEntity, "����/ȼ������������", "ȼ������");
			//Sayϵͳ(oss.str());
			m_list�ȴ���.clear();
			co_return{};
		}
		refSpace.GetSpacePlayer(m_refEntity).m_u32ȼ���� -= ����.����.u16����ȼ����;

		if (����.����.u16���ľ���� > Space::GetSpacePlayer(m_refEntity).m_u32�����)
		{
			//refGateSession.m_u32ȼ���� += ����.����.u16����ȼ����;
			PlayerComponent::��������(m_refEntity, "����/�������������", "�������");
			m_list�ȴ���.clear();
			co_return{};
		}
		Space::GetSpacePlayer(m_refEntity).m_u32����� -= ����.����.u16���ľ����;

		if (Is�׳�())
			m_refEntity.BroadcastChangeSkeleAnim("Armature|Armature.003|Take 001|BaseLayer");

		const int MAX���� = 10;
		for (int i = 0; i < 10; ++i)
		{
			if (co_await CoTimer::Wait(300ms, m_TaskCancel����λ.cancel))
			{
				m_list�ȴ���.clear();
				co_return{};
			}
			if (Is�׳�())
				EntitySystem::BroadcastEntity����(m_refEntity, std::format("�ɱ����{0}/{1}", i, MAX����));
			else
				EntitySystem::BroadcastEntity����(m_refEntity, std::format("�������{0},��ǰ��λ����{1}/{2}", m_list�ȴ���.size(), i, MAX����));
		}

		//LOG(INFO) << "Э��RPC����,error=" << responce.error << ",finalMoney=" << responce.finalMoney;
		//CHECK_CO_RET_0(!refGateSession.m_wpSpace.expired());
		auto wpNewEntity = m_refEntity.m_refSpace.����λ(m_refEntity.m_spPlayer, EntitySystem::GetNickName(m_refEntity), pos, ����);
		CHECK_WP_CO_RET_FALSE(wpNewEntity);
		auto& ref��Դ = *wpNewEntity.lock();
		//if (m_list�ȴ���.empty())
		//{
		//	LOG(ERROR) << "err";
		//	_ASSERT(false);
		//	co_return{};
		//}
		if (m_pos����� != m_refEntity.Pos())
		{
			if (!�ɼ�����㸽������Դ(ref��Դ))
			{
				CHECK_CO_RET_FALSE(ref��Դ.m_sp��);
				auto pos = m_pos�����;
				m_refEntity.m_refSpace.CrowdToolFindNerestPos(pos);
				ref��Դ.m_sp��->WalkToPos(pos);
			}
		}
	}

	using namespace std;
	if (�׳� == m_refEntity.m_����)
		m_refEntity.CoDelayDelete(1ms).RunNew();
	else
		EntitySystem::BroadcastEntity����(m_refEntity, "������");
}

bool ����λComponent::�ɼ�����㸽������Դ(Entity& refEntiy)const
{
	if (!refEntiy.m_sp�ɼ�)
		return false;//���� ���̳� �� ����

	const auto [i32����Id, i32����X, i32����Z] = AoiComponent::����(m_pos�����);
	const auto& mapEntity = m_refEntity.m_refSpace.m_map����һ����[i32����Id];
	const auto iterFind = std::find_if(mapEntity.begin(), mapEntity.end(), [](const auto& pair)
		{
			CHECK_WP_RET_FALSE(pair.second);
			const auto& refEntity = *pair.second.lock();
			return EntitySystem::Is��Դ(refEntity.m_����);
		});
	if (mapEntity.end() == iterFind)
		return false;

	CHECK_WP_RET_FALSE(iterFind->second);
	const auto& wp��Դ = iterFind->second;
	if (!wp��Դ.lock()->Pos().DistanceLessEqual(m_pos�����, 1))
		return false;

	refEntiy.m_sp�ɼ�->�ɼ�(wp��Դ);
	return true;
}

uint16_t ����λComponent::�ȴ���Count()const
{
	return (uint16_t)m_list�ȴ���.size();
}