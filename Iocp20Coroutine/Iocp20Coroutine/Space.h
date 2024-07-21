#pragma once
#include <set>
class Entity;
class MyServer;
/// <summary>
/// GoÓïÑÔGoWorld¿ò¼ÜSpace/Entity£¬Ïàµ±ÓÚC#ÓïÑÔET¿ò¼ÜScene/Unit
/// </summary>
class Space
{
public:
	Space(MyServer* pServer) :m_pServer(pServer) {}
	std::set<Entity*> setEntity;
	void Update();
	MyServer * const m_pServer;
};