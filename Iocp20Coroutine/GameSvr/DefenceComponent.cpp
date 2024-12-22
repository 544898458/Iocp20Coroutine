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
	refEntity.m_spDefence = std::make_shared<DefenceComponent, Entity&, const int>(refEntity,u16��ʼHp);
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
		if (m_refEntity.m_spAttack)
		{
			EntitySystem::Broadcast��������(m_refEntity, m_refEntity.m_spAttack->m_ս������.str������Ч);
			m_refEntity.BroadcastChangeSkeleAnim(m_refEntity.m_spAttack->m_ս������.str��������, false);//������������
			//switch (m_refEntity.m_spAttack->m_����)
			//{
			//case ��:
			//	EntitySystem::Broadcast��������(m_refEntity, "TMaDth00"); 
			//	m_refEntity.BroadcastChangeSkeleAnim(m_refEntity.m_spPlayer?"die01":"died", false);//������������
			//	break;
			//case ��ս��:
			//	EntitySystem::Broadcast��������(m_refEntity, "TFbDth00"); 
			//	m_refEntity.BroadcastChangeSkeleAnim("died", false);//������������
			//	break;
			//case ���̳�:
			//	EntitySystem::Broadcast��������(m_refEntity, "TSCDth00"); 
			//	m_refEntity.BroadcastChangeSkeleAnim("died", false);//������������
			//	break;
			//default:
			//	break;
			//}
		}
		else if (m_refEntity.m_spBuilding)
		{
			switch (m_refEntity.m_spBuilding->m_����)
			{
			case ����:
				EntitySystem::Broadcast��������(m_refEntity, "explo4");
				m_refEntity.BroadcastChangeSkeleAnim("die", false);//������������
				break;
			//case ����:EntitySystem::Broadcast��������(""); break;
			//case ��:EntitySystem::Broadcast��������(""); break;
			//case �ر�:EntitySystem::Broadcast��������("explo4"); break;
			default:
				EntitySystem::Broadcast��������(m_refEntity, "EXPLOMED"); break;
				break;
			}
		}
		m_refEntity.CoDelayDelete().RunNew();
		CoEvent<MyEvent::��λ����>::OnRecvEvent(false, { .wpEntity = m_refEntity.weak_from_this() });
		PlayerComponent::Send��Դ(m_refEntity);
	}
}


bool DefenceComponent::IsDead() const
{
	return m_hp <= 0;
}
