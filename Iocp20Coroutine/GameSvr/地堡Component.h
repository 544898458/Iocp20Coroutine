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
	static void AddComponet(Entity& refEntity);
	void OnDestroy();
	void OnBeforeDelayDelete();
	void ��(Space& refSpace, uint64_t idEntity);
	void Update();
	void ȫ�����ر�();
	Entity& m_refEntity;
	std::list<SpEntity> m_listSpEntity;
};

