#pragma once
#include "../CoRoutine/CoTask.h"
class Entity;
class MonsterComponent
{
public:
	MonsterComponent(Entity&);
//private:
	CoTask<int> m_coIdle;
};

