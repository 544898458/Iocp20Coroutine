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

	播放正遭到攻击语音();
	m_refEntity.BroadcastNotifyPos();
	if (IsDead())
	{
		单位::单位配置 单位;
		if (单位::Find单位配置(m_refEntity.m_类型, 单位))
		{
			EntitySystem::Broadcast播放声音(m_refEntity, 单位.str阵亡音效);
			m_refEntity.BroadcastChangeSkeleAnim(单位.str阵亡动作, false);//播放阵亡动作
		}
		else
		{
			LOG(WARNING) << "没有类型:" << m_refEntity.m_类型;
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

void DefenceComponent::播放正遭到攻击语音()
{
	using namespace std;
	const auto now = std::chrono::system_clock::now();
	if (now - m_time上次提醒正遭到攻击 < 60s)
		return;

	m_time上次提醒正遭到攻击 = now;

	LPCSTR sz语音 = "";
	switch (m_refEntity.m_类型)
	{
	case 基地:sz语音 = "语音/基地正遭到进攻女声正经版"; break;
	case 工程车:sz语音 = "语音/工程车遭到攻击女声可爱版"; break;
	default:
		return;
		break;
	}
	PlayerComponent::播放声音(m_refEntity, sz语音);
}
