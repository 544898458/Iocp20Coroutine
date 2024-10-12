#include "pch.h"
#include "MonsterComponent.h"
#include "Entity.h"
#include "AiCo.h"
#include "AttackComponent.h"
#include "Space.h"

void Entity::AddComponentMonster()
{
	m_spMonster = std::make_shared<MonsterComponent, Entity&>(*this);
}

MonsterComponent::MonsterComponent(Entity& refThis)
{
	if (!refThis.m_spAttack)
	{
		LOG(ERROR) << "m_spAttack";
		return;
	}
	m_coIdle = AiCo::Idle(refThis.shared_from_this(), refThis.m_spAttack->m_cancel);
	m_coIdle.Run();
}

void MonsterComponent::AddMonster(Space& refSpace)
{
	SpEntity spEntityMonster = std::make_shared<Entity, const Position&, Space&, const std::string&, const std::string& >({ -30.0 }, refSpace, "altman-red", "¹Ö");
	spEntityMonster->AddComponentAttack();
	spEntityMonster->AddComponentMonster();
	spEntityMonster->m_f¾¯½ä¾àÀë = 20;
	spEntityMonster->m_fÒÆ¶¯ËÙ¶È = 0.2f;
	refSpace.m_mapEntity.insert({ (int64_t)spEntityMonster.get() ,spEntityMonster });
	//LOG(INFO) << "SpawnMonster:" << refSpace.m_mapEntity.size();
	spEntityMonster->BroadcastEnter();
}