#pragma once
#include "SpEntity.h"
#include <list>
class Entity;
class PlayerGateSession_Game;
class Space;

class �ر�Component
{
public:
	�ر�Component(Entity &ref):m_refEntity(ref){}
	static void AddComponent(Entity& refEntity);
	void OnDestroy();
	void OnBeforeDelayDelete();
	void ��(Space& refSpace, Entity& refEntity);
	void ȫ�����ر�();
	Entity& m_refEntity;
	std::list<SpEntity> m_listSpEntity;
};

