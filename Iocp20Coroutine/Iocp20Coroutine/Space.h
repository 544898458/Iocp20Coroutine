#pragma once
#include <set>
class Entity;
class MyServer;
/// <summary>
/// Go����GoWorld���Space/Entity���൱��C#����ET���Scene/Unit
/// </summary>
class Space
{
public:
	Space(MyServer* pServer) :m_pServer(pServer) {}
	std::set<Entity*> setEntity;
	void Update();
	MyServer * const m_pServer;
};