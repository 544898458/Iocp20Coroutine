#pragma once
#include "../CoRoutine/CoTask.h"
#include "MyEvent.h"

class Space;
struct Position;
class MonsterComponent
{
public:
	MonsterComponent(Entity&);
	static std::vector<SpEntity> AddMonster(Space& refSpace, const Position& refPos, const int count = 1);
	static void AddComponent(Entity& refEntity);
	//private:
	//CoTask<int> m_coIdle;
};

