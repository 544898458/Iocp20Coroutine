#pragma once
#include "SpEntity.h"
#include <list>
class Entity;
class PlayerGateSession_Game;
class Space;

class 可进活动单位Component
{
public:
	可进活动单位Component(Entity &ref):m_refEntity(ref){}
	static void AddComponent(Entity& refEntity);
	void OnEntityDestroy(const bool bDestroy);
	void OnBeforeDelayDelete();
	void 进(Space& refSpace, Entity& refEntity);
	void 全都出去();
	Entity& m_refEntity;
	std::list<SpEntity> m_listSpEntity;
};

