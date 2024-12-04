#include "pch.h"
#include "�콨��Component.h"
#include "��λ.h"
#include "AiCo.h"
#include "Entity.h"
#include "EntitySystem.h"
#include "PlayerComponent.h"
#include "AttackComponent.h"
#include "PlayerGateSession_Game.h"
#include "../CoRoutine/CoTimer.h"
#include "../IocpNetwork/StrConv.h"

void �콨��Component::AddComponent(Entity& refEntity, PlayerGateSession_Game& refGateSession, const ���λ���� ����)
{
	refEntity.m_sp�콨�� = std::make_shared<�콨��Component, PlayerGateSession_Game&, Entity&, const ���λ����>(refGateSession, refEntity, std::forward<const ���λ����&&>(����));
}

bool �콨��Component::���ڽ���(Entity& refEntity)
{
	if (!refEntity.m_sp�콨��)
		return false;

	return refEntity.m_sp�콨��->m_cancel�콨��.operator bool();
}

�콨��Component::�콨��Component(PlayerGateSession_Game& refSession, Entity& refEntity, const ���λ���� ����) :m_refEntity(refEntity)
{
	switch (����)
	{
	case ���̳�:
		m_set��������.insert(����);
		m_set��������.insert(��);
		m_set��������.insert(����);
		m_set��������.insert(�ر�);
		break;
	default:
		break;
	}
}

CoTaskBool �콨��Component::Co�콨��(const Position refPos, const ������λ���� ����)
{
	if (m_set��������.end() == m_set��������.find(����))
	{
		co_return false;
	}

	PlayerComponent::��������(m_refEntity, "TSCYes01");

	//���ߵ�Ŀ���
	if (co_await AiCo::WalkToPos(m_refEntity, refPos, m_cancel�콨��, 5))
		co_return true;

	//Ȼ��ʼ��Ǯ����
	auto spEntity���� = co_await m_refEntity.m_spPlayer->m_refSession.CoAddBuilding(����, refPos);
	if (!spEntity����)
		co_return false;

	if (co_await Co�������(spEntity����, m_cancel�콨��))
		co_return true;

	co_return false;
}

CoTaskBool �콨��Component::Co�������(WpEntity wpEntity����, FunCancel& cancel)
{
	PlayerComponent::��������(m_refEntity, "TSCTra00");

	KeepCancel kc(cancel);
	std::weak_ptr<BuildingComponent> wpBuilding(wpEntity����.lock()->m_spBuilding);

	while (!wpBuilding.expired() && MAX����ٷֱ� > wpBuilding.lock()->m_n������Ȱٷֱ�)
	{
		if (co_await CoTimer::WaitNextUpdate(cancel))
			co_return true;
		if (wpBuilding.expired())
			co_return true;

		BuildingComponent& refBuilding = *wpBuilding.lock();
		++refBuilding.m_n������Ȱٷֱ�;

		std::ostringstream oss;
		if (MAX����ٷֱ� <= refBuilding.m_n������Ȱٷֱ�)
			oss << "�������";
		else
			oss << "���ڽ���:" << refBuilding.m_n������Ȱٷֱ� << "%";

		assert(!wpEntity����.expired());
		EntitySystem::BroadcastEntity����(*wpEntity����.lock(), oss.str());
		if (m_refEntity.m_spPlayer)
			m_refEntity.m_spPlayer->m_refSession.Send��Դ();
	}

	if (m_refEntity.m_spAttack)
	{
		switch (m_refEntity.m_spAttack->m_����)
		{
		case ���̳�:PlayerComponent::��������(m_refEntity, "TSCUpd00"); break;
		default:break;
		}
	}
	co_return 0;
}

void �콨��Component::TryCancel()
{
	if (m_cancel�콨��)
	{
		//LOG(INFO) << "����m_cancel";
		m_cancel�콨��();
	}
	else
	{

	}
}