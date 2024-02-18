#pragma once
#include <map>
class Entity;
/// <summary>
/// GoÓïÑÔGoWorld¿ò¼ÜSpace/Entity£¬Ïàµ±ÓÚC#ÓïÑÔET¿ò¼ÜScene/Unit
/// </summary>
class Space
{
public:
	std::map<int, Entity*> mapEntity;
	void Update();
};
extern Space g_space;