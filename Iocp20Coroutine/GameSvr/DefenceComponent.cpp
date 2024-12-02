#include "pch.h"
#include "DefenceComponent.h"
#include "Entity.h"
#include "EntitySystem.h"
#include "AiCo.h"
#include "../CoRoutine/CoEvent.h"
#include "MyEvent.h"
#include "AttackComponent.h"

DefenceComponent::DefenceComponent(Entity& refEntity) :m_refEntity(refEntity)
{
}

void DefenceComponent::AddComponent(Entity& refEntity, uint16_t u16��ʼHp)
{
	CHECK_VOID(!refEntity.m_spDefence);
	refEntity.m_spDefence = std::make_shared<DefenceComponent, Entity&>(refEntity);
	refEntity.m_spDefence->m_hp = u16��ʼHp;
}


void DefenceComponent::����(int hp)
{
	CHECK_GE(hp, 0);
	if (IsDead())
		return;

	this->m_hp -= hp;

	m_refEntity.BroadcastNotifyPos();
	if (IsDead())
	{
		m_refEntity.BroadcastChangeSkeleAnim("died", false);//������������
		if (m_refEntity.m_spAttack)
		{
			switch (m_refEntity.m_spAttack->m_����)
			{
			case ��:EntitySystem::Broadcast��������(m_refEntity, "TMaDth00"); break;//Standing by. ������
			case ��ս��:EntitySystem::Broadcast��������(m_refEntity, "TFbDth00"); break;//Checked up and good to go. �����ϣ�׼������
			case ���̳�:EntitySystem::Broadcast��������(m_refEntity, "TSCDth00"); break;
			default:
				break;
			}
		}
		m_refEntity.CoDelayDelete().RunNew();
		CoEvent<MyEvent::��λ����>::OnRecvEvent(false, { .wpEntity = m_refEntity.weak_from_this() });
	}
}


bool DefenceComponent::IsDead() const
{
	return m_hp <= 0;
}
