#include "pch.h"
#include "�ɼ�Component.h"
#include "Entity.h"

void �ɼ�Component::AddComponent(Entity& refEntity)
{
	refEntity.m_sp�ɼ�= std::make_shared<�ɼ�Component>();
}
