#include "stdafx.h"
#include "BuildingComponent.h"
#include "Entity.h"
#include "../CoRoutine/CoRpc.h"
#include "AiCo.h"

void Entity::AddComponentBuilding()
{
	m_spBuilding = std::make_shared<BuildingComponent, Entity&>(*this);
}

BuildingComponent::BuildingComponent(Entity& refEntity)
{
	m_coAddMoney = AiCo::AddMoney(refEntity.shared_from_this(), m_cancel);
	m_coAddMoney.Run();
}