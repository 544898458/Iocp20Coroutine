#pragma once
#include "SpEntity.h"
#include <list>
class Entity;
class PlayerGateSession_Game;
class Space;

class �ɽ����λComponent
{
public:
	�ɽ����λComponent(Entity &ref):m_refEntity(ref){}
	static void AddComponent(Entity& refEntity);
	void OnEntityDestroy(const bool bDestroy);
	void OnBeforeDelayDelete();
	void ��(Space& refSpace, Entity& refEntity);
	void ȫ����ȥ();
	Entity& m_refEntity;
	std::list<SpEntity> m_listSpEntity;
};

