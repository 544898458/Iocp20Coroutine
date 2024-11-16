#include "pch.h"
#include "DefenceComponent.h"
#include "Entity.h"
#include "AiCo.h"
#include "../CoRoutine/CoEvent.h"
#include "MyEvent.h"

DefenceComponent::DefenceComponent(Entity& refEntity) :m_refEntity(refEntity)
{
}

void DefenceComponent::AddComponent(Entity& refEntity, uint16_t u16初始Hp)
{
	CHECK_VOID(!refEntity.m_spDefence);
	refEntity.m_spDefence = std::make_shared<DefenceComponent, Entity&>(refEntity);
	refEntity.m_spDefence->m_hp = u16初始Hp;
}


void DefenceComponent::受伤(int hp)
{
	CHECK_GE(hp, 0);
	if (IsDead())
		return;

	this->m_hp -= hp;

	m_refEntity.BroadcastNotifyPos();
	if (IsDead())
	{
		m_refEntity.BroadcastChangeSkeleAnim("died", false);//播放死亡动作
		m_refEntity.CoDelayDelete().RunNew();
		CoEvent<MyEvent::单位阵亡>::OnRecvEvent(false, { .wpEntity = m_refEntity.weak_from_this() });
	}
}


bool DefenceComponent::IsDead() const
{
	return m_hp <= 0;
}
