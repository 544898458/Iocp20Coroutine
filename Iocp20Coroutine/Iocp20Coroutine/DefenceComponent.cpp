#include "pch.h"
#include "DefenceComponent.h"
#include "Entity.h"

void DefenceComponent::AddComponent(Entity& refEntity)
{
	CHECK_VOID(!refEntity.m_spDefence);
	refEntity.m_spDefence = std::make_shared<DefenceComponent>();
}
