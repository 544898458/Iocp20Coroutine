#pragma once
#include "../CoRoutine/CoTask.h"
#include "MyEvent.h"

class Space;
class MonsterComponent
{
public:
	MonsterComponent(Entity&);
	static void AddMonster(Space& refSpace, const int count = 1);
	static void AddComponent(Entity& refEntity);
	//private:
	CoTask<int> m_coIdle;
};

