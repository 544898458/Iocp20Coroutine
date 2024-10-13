#include "pch.h"
#include "采集Component.h"
#include "Entity.h"

void 采集Component::AddComponent(Entity& refEntity)
{
	refEntity.m_sp采集= std::make_shared<采集Component>();
}
