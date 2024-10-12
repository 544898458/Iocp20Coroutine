#pragma once
#include "../CoRoutine/CoTask.h"
#include "MyEvent.h"

class Space;
class MonsterComponent
{
public:
	MonsterComponent(Entity&);
	static void AddMonster(Space& refSpace);
//private:
	CoTask<int> m_coIdle;
};

