#pragma once
#include <map>
class Entity;
/// <summary>
/// Go����GoWorld���Space/Entity���൱��C#����ET���Scene/Unit
/// </summary>
class Space
{
public:
	std::map<int, Entity*> mapEntity;
	void Update();
};
extern Space g_space;