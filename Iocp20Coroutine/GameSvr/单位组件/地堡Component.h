#pragma once
#include "SpEntity.h"
#include <list>
class Entity;
class PlayerGateSession_Game;
class Space;

class 地堡Component
{
public:
	地堡Component(Entity &ref):m_refEntity(ref){}
	static void AddComponent(Entity& refEntity);
	void OnDestroy();
	void OnBeforeDelayDelete();
	void 进(Space& refSpace, Entity& refEntity);
	void 全都出地堡();
	Entity& m_refEntity;
	std::list<SpEntity> m_listSpEntity;
};

