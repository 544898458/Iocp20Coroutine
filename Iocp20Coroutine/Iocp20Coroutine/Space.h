#pragma once
#include <set>
#include "SpEntity.h"

class MyServer;
/// <summary>
/// Go����GoWorld���Space/Entity���൱��C#����ET���Scene/Unit
/// </summary>
class Space
{
public:
	Space(MyServer* pServer) :m_pServer(pServer) {}
	std::set<SpEntity> setEntity;
	void Update();
	MyServer * const m_pServer;
};