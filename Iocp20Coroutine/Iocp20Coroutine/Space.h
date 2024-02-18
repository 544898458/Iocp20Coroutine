#pragma once
#include <map>
class Entity;
class Space
{
public:
	std::map<int, Entity*> mapEntity;
	void Update();
};
extern Space space;