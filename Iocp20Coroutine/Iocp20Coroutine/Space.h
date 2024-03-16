#pragma once
#include <set>
class Entity;
/// <summary>
/// GoÓïÑÔGoWorld¿ò¼ÜSpace/Entity£¬Ïàµ±ÓÚC#ÓïÑÔET¿ò¼ÜScene/Unit
/// </summary>
class Space
{
public:
	std::set<Entity*> setEntity;
	void Update();
};