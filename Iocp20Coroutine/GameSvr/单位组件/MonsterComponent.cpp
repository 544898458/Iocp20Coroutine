#include "pch.h"
#include "MonsterComponent.h"
#include "Entity.h"
#include "AiCo.h"
#include "AttackComponent.h"
#include "DefenceComponent.h"
#include "Space.h"
#include "��Component.h"
#include "��λ.h"

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
	��λ::������ ����;
	if (!��λ::Find������(����, ����))
	{
		LOG(ERROR) << "û�й�����:" << ����;
		return {};
	}
	for (int i = 0; i < count; ++i)
	{
		SpEntity spEntityMonster = std::make_shared<Entity, const Position&, Space&, const ��λ����, const ��λ::��λ����&>(
			refPos, refSpace, std::forward<const ��λ����&&>(����), ����.����);
		AttackComponent::AddComponent(*spEntityMonster, ����, ����.ս��);
		DefenceComponent::AddComponent(*spEntityMonster, ����.u16��ʼHp);
		��Component::AddComponent(*spEntityMonster);
		AddComponent(*spEntityMonster);
		//spEntityMonster->m_f������� = ����.ս��.f�������;
		spEntityMonster->m_�ٶ�ÿ֡�ƶ����� = ����.ս��.fÿ֡�ƶ�����;
		//refSpace.m_mapEntity.insert({ (int64_t)spEntityMonster.get() ,spEntityMonster });
		refSpace.AddEntity(spEntityMonster);
		//LOG(INFO) << "SpawnMonster:" << refSpace.m_mapEntity.size();
		spEntityMonster->BroadcastEnter();
		vecRet.emplace_back(spEntityMonster);
	}
	return vecRet;
}