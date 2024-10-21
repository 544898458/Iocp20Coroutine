#pragma once
#include "SpEntity.h"
#include <list>
class Entity;
class PlayerGateSession_Game;
class Space;

class µØ±¤Component
{
public:
	static void AddComponet(Entity& refEntity, PlayerGateSession_Game& refGateSession);
	void ½ø(Space& refSpace, uint64_t idEntity);
	std::list<SpEntity> m_listSpEntity;
};

