#pragma once
#include "SpEntity.h"
#include <list>
class Entity;
class PlayerGateSession_Game;
class Space;

class �ر�Component
{
public:
	static void AddComponet(Entity& refEntity, PlayerGateSession_Game& refGateSession);
	void ��(Space& refSpace, uint64_t idEntity);
	void Update();
	std::list<SpEntity> m_listSpEntity;
};

