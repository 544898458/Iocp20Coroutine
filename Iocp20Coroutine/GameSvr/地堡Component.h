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
	static void AddComponet(Entity& refEntity);
	void OnDestroy();
	void OnBeforeDelayDelete();
	void 进(Space& refSpace, uint64_t idEntity);
	void Update();
	void 全都出地堡();
	Entity& m_refEntity;
	std::list<SpEntity> m_listSpEntity;
};

