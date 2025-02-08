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

void DefenceComponent::AddComponent(Entity& refEntity, uint16_t u16初始Hp)
{
	CHECK_VOID(!refEntity.m_spDefence);
	refEntity.m_spDefence = std::make_shared<DefenceComponent, Entity&, const int>(refEntity, u16初始Hp);
}


void DefenceComponent::受伤(int hp, const uint64_t idAttacker)
{
	CHECK_GE(hp, 0);
	if (IsDead())
		return;

	this->m_hp -= hp;
	m_map对我伤害[idAttacker] += hp;

	播放基地正遭到攻击语音();
	m_refEntity.BroadcastNotifyPos();
	if (IsDead())
	{
		if (m_refEntity.m_spAttack)
		{
			EntitySystem::Broadcast播放声音(m_refEntity, m_refEntity.m_spAttack->m_战斗配置.str阵亡音效);
			m_refEntity.BroadcastChangeSkeleAnim(m_refEntity.m_spAttack->m_战斗配置.str阵亡动作, false);//播放阵亡动作
		}
		else if (m_refEntity.m_spBuilding)
		{
			switch (m_refEntity.m_spBuilding->m_类型)
			{
			case 基地:
				EntitySystem::Broadcast播放声音(m_refEntity, "explo4");
				m_refEntity.BroadcastChangeSkeleAnim("die", false);//播放死亡动作
				break;
			default:
				EntitySystem::Broadcast播放声音(m_refEntity, "EXPLOMED"); break;
				break;
			}
		}
		m_refEntity.CoDelayDelete().RunNew();
		CoEvent<MyEvent::单位阵亡>::OnRecvEvent({ .wpEntity = m_refEntity.weak_from_this() });
		PlayerComponent::Send资源(m_refEntity);
	}
}


bool DefenceComponent::IsDead() const
{
	return m_hp <= 0;
}

void DefenceComponent::播放基地正遭到攻击语音()
{
	if (m_refEntity.m_spBuilding)
	{
		switch (m_refEntity.m_spBuilding->m_类型)
		{
		case 基地:
		{
			const auto now = std::chrono::system_clock::now();
			using namespace std;
			if (now - m_time上次提醒基地正遭到进攻 > 60s)
			{
				m_time上次提醒基地正遭到进攻 = now;
				PlayerComponent::播放声音(m_refEntity, "语音/基地正遭到进攻女声正经版");
			}
			break;
		}
		default:

			break;
		}
	}
}
