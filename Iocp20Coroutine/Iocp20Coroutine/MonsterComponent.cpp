#include "stdafx.h"
#include "MonsterComponent.h"
#include "Entity.h"
#include "AiCo.h"

void Entity::AddComponentMonster()
{
	m_spMonster = std::make_shared<MonsterComponent, Entity&>(*this);
}

MonsterComponent::MonsterComponent(Entity& refEntity)
{
	m_coIdle = AiCo::Idle(refEntity.shared_from_this(), refEntity.m_cancel);
	m_coIdle.Run();
}