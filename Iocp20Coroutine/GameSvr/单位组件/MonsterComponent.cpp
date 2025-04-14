#include "pch.h"
#include "MonsterComponent.h"
#include "Entity.h"
#include "AiCo.h"
#include "AttackComponent.h"
#include "DefenceComponent.h"
#include "Space.h"
#include "��Component.h"
#include "��λ.h"
#include "../EntitySystem.h"

void MonsterComponent::AddComponent(Entity& refEntity)
{
	refEntity.m_spMonster = std::make_shared<MonsterComponent, Entity&>(refEntity);
}

MonsterComponent::MonsterComponent(Entity& refThis)
{
	if (!refThis.m_spAttack)
	{
		LOG(ERROR) << "m_spAttack";
		return;
	}
	//m_coIdle = AiCo::Idle(refThis.shared_from_this(), refThis.m_spAttack->m_cancel);
	//m_coIdle.Run();
}


std::vector<SpEntity> MonsterComponent::AddMonster(Space& refSpace, const ��λ���� ����, const Position& refPos, const int count)
{
	std::vector<SpEntity> vecRet;
	��λ::������ ������;
	��λ::��λ���� ��λ����;
	��λ::ս������ ս������;
	CHECK_RET_DEFAULT(��λ::Find������(����, ������));
	CHECK_RET_DEFAULT(��λ::Find��λ����(����, ��λ����));
	CHECK_RET_DEFAULT(��λ::Findս������(����, ս������));
	for (int i = 0; i < count; ++i)
	{
		SpEntity spEntityMonster = std::make_shared<Entity, const Position&, Space&, const ��λ����, const ��λ::��λ����&>(
			refPos, refSpace, std::forward<const ��λ����&&>(����), ��λ����);
		AttackComponent::AddComponent(*spEntityMonster).m_fun��������˴� = AttackComponent::�����й�;

		DefenceComponent::AddComponent(*spEntityMonster, ������.u16��ʼHp);
		��Component::AddComponent(*spEntityMonster);
		AddComponent(*spEntityMonster);
		
		
		refSpace.AddEntity(spEntityMonster);
		//LOG(INFO) << "SpawnMonster:" << refSpace.m_mapEntity.size();
		spEntityMonster->BroadcastEnter();
		vecRet.emplace_back(spEntityMonster);
	}
	return vecRet;
}