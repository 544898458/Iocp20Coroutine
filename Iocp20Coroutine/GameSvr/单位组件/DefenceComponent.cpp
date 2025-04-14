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
#include "../Space.h"

DefenceComponent::DefenceComponent(Entity& refEntity, const int i32HpMax) : m_refEntity(refEntity), m_i32HpMax(i32HpMax), m_hp(i32HpMax)
{
}

void DefenceComponent::AddComponent(Entity& refEntity, uint16_t u16��ʼHp)
{
	CHECK_VOID(!refEntity.m_spDefence);
	refEntity.m_spDefence = std::make_shared<DefenceComponent, Entity&, const int>(refEntity, u16��ʼHp);
}

uint16_t DefenceComponent::������ķ���(Entity& refEntity)
{
	//uint16_t u16�ȼ�(0);
	//if (refEntity.m_spPlayerNickName)
	//{
	//	const auto& spacePlayer = refEntity.m_refSpace.GetSpacePlayer(refEntity);
	//	u16�ȼ� = spacePlayer.��λ���Եȼ�(refEntity.m_����, ����);
	//}

	//return ��λ::��λ����(refEntity.m_����, u16�ȼ�);
	return EntitySystem::���������(refEntity);
}
void DefenceComponent::����(const int ����, const uint64_t idAttacker)
{
	CHECK_GE(����, 0);
	if (IsDead())
		return;

	const auto ���� = ������ķ���(m_refEntity);
	const auto �˺� = std::max(���� - ����, 1);

	this->m_hp -= �˺�;
	m_map�����˺�[idAttacker] += �˺�;

	�������⵽��������();
	m_refEntity.BroadcastNotifyPos();
	if (IsDead())
	{
		��λ::��λ���� ��λ;
		if (��λ::Find��λ����(m_refEntity.m_����, ��λ))
		{
			EntitySystem::Broadcast��������(m_refEntity, ��λ.str������Ч);
			m_refEntity.BroadcastChangeSkeleAnim(��λ.str��������, false);//������������
		}
		else
		{
			LOG(WARNING) << "û������:" << m_refEntity.m_����;
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
	case ����:sz���� = "����/�������⵽����Ů��������"; break;
	case ���̳�:sz���� = "����/���̳��⵽����Ů���ɰ���"; break;
	case �泲:sz���� = "����/�泲���⵽����_����"; break;
	case ����:sz���� = "����/�����⵽����_����"; break;
	default:
		return;
		break;
	}
	PlayerComponent::��������(m_refEntity, sz����, m_refEntity.m_����.strName + " �⵽����");
}

bool DefenceComponent::����Ѫ() const
{
	return m_hp >= m_i32HpMax;
}
