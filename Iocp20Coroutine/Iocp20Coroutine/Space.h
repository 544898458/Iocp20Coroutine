#pragma once
#include <set>
class Entity;
/// <summary>
/// Go����GoWorld���Space/Entity���൱��C#����ET���Scene/Unit
/// </summary>
class Space
{
public:
	std::set<Entity*> setEntity;
	void Update();
};