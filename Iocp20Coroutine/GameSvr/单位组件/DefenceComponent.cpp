#include "pch.h"
#include "DefenceComponent.h"
#include "Entity.h"
#include "EntitySystem.h"
#include "AiCo.h"
#include "../CoRoutine/CoEvent.h"
#include "MyEvent.h"
#include "AttackComponent.h"
#include "PlayerComponent.h"
#include "BuildingComponent.h"

DefenceComponent::DefenceComponent(Entity& refEntity, const int i32HpMax) : m_refEntity(refEntity), m_i32HpMax(i32HpMax), m_hp(i32HpMax)
{
}

void DefenceComponent::AddComponent(Entity& refEntity, uint16_t u16��ʼHp)
{
	CHECK_VOID(!refEntity.m_spDefence);
	refEntity.m_spDefence = std::make_shared<DefenceComponent, Entity&, const int>(refEntity, u16��ʼHp);
}


void DefenceComponent::����(int hp, const uint64_t idAttacker)
{
	CHECK_GE(hp, 0);
	if (IsDead())
		return;

	this->m_hp -= hp;
	m_map�����˺�[idAttacker] += hp;

	�������⵽��������();
	m_refEntity.BroadcastNotifyPos();
	if (IsDead())
	{
		if (m_refEntity.m_spAttack)
		{
			EntitySystem::Broadcast��������(m_refEntity, m_refEntity.m_spAttack->m_ս������.str������Ч);
			m_refEntity.BroadcastChangeSkeleAnim(m_refEntity.m_spAttack->m_ս������.str��������, false);//������������
		}
		else if (m_refEntity.m_spBuilding)
		{
			switch (m_refEntity.m_����)
			{
			case ����:
				EntitySystem::Broadcast��������(m_refEntity, "explo4");
				m_refEntity.BroadcastChangeSkeleAnim("die", false);//������������
				break;
			case ������:
				EntitySystem::Broadcast��������(m_refEntity, "explo4");
				m_refEntity.BroadcastChangeSkeleAnim("Take 001", false);
				break;
			case ����:
				EntitySystem::Broadcast��������(m_refEntity, "explo4");
				m_refEntity.BroadcastChangeSkeleAnim("�������", false);
				break;
			case ������:
				EntitySystem::Broadcast��������(m_refEntity, "explo4");
				m_refEntity.BroadcastChangeSkeleAnim("����������", false);
				break;
			default:
				EntitySystem::Broadcast��������(m_refEntity, "EXPLOMED"); break;
				break;
			}
		}
		m_refEntity.CoDelayDelete().RunNew();
		CoEvent<MyEvent::��λ����>::OnRecvEvent({ .wpEntity = m_refEntity.weak_from_this() });
		PlayerComponent::Send��Դ(m_refEntity);
	}
}


bool DefenceComponent::IsDead() const
{
	return m_hp <= 0;
}

void DefenceComponent::�������⵽��������()
{
	using namespace std;
	const auto now = std::chrono::system_clock::now();
	if (now - m_time�ϴ��������⵽���� < 60s)
		return;

	m_time�ϴ��������⵽���� = now;

	LPCSTR sz���� = "";
	switch (m_refEntity.m_����)
	{
	case ����:sz���� = "����/�������⵽����Ů��������";break;
	case ���̳�:sz���� = "����/���̳��⵽����Ů���ɰ���"; break;
	default:
		return;
		break;
	}
	PlayerComponent::��������(m_refEntity, sz����);
}
