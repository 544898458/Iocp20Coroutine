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
#include "��ʱ�赲Component.h"
#include "��ֵComponent.h"
#include "../Space.h"
#include "../ö��/��������.h"

DefenceComponent::DefenceComponent(Entity& refEntity, const uint16_t i16HpMax) : m_refEntity(refEntity)
{
	��ֵComponent::Set(refEntity, ����, i16HpMax);
	��ֵComponent::Set(refEntity, ��������::�������, i16HpMax);
}

void DefenceComponent::AddComponent(Entity& refEntity, uint16_t u16��ʼHp)
{
	��ֵComponent::AddComponent(refEntity);
	CHECK_RET_VOID(!refEntity.m_upDefence);
	refEntity.m_upDefence.reset(new DefenceComponent(refEntity, u16��ʼHp));
}

uint16_t DefenceComponent::������ķ���(Entity& refEntity)
{
	//uint16_t u16�ȼ�(0);
	//if (refEntity.m_upPlayerNickName)
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

	��ֵComponent::�ı�(m_refEntity, ����, -�˺�); //this->m_hp -= �˺�;
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
		m_refEntity.DelayDelete();
		CoEvent<MyEvent::��λ����>::OnRecvEvent({ .wpEntity = m_refEntity.weak_from_this() });
		PlayerComponent::Send��Դ(m_refEntity);
        if (m_refEntity.m_up��ʱ�赲)
        {
			m_refEntity.m_up��ʱ�赲->OnEntityDestroy(false);
        }
	}
}


bool DefenceComponent::IsDead() const
{
	return ��ֵComponent::Get(m_refEntity, ����) <= 0;
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

int DefenceComponent::�������()const
{
	return ��ֵComponent::Get(m_refEntity, ��������::�������);
}

bool DefenceComponent::����Ѫ() const
{
	return �������() <= ��ֵComponent::Get(m_refEntity, ����);
}

void DefenceComponent::��Ѫ(int16_t i16�仯)
{
	if (0 == i16�仯)
		return;

	CHECK_RET_VOID(i16�仯 > 0);
	const auto old = ��ֵComponent::Get(m_refEntity, ����);
	if (old >= �������())
		return;

	��ֵComponent::Set(m_refEntity, ����, std::min(�������(), old + i16�仯));

	if (old != ��ֵComponent::Get(m_refEntity, ����))
		m_refEntity.BroadcastNotifyPos();
}