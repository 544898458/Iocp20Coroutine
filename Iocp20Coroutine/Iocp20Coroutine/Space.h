#pragma once
#include <set>
#include "SpEntity.h"

class MyServer;
/// <summary>
/// GoÓïÑÔGoWorld¿ò¼ÜSpace/Entity£¬Ïàµ±ÓÚC#ÓïÑÔET¿ò¼ÜScene/Unit
/// </summary>
class Space
{
public:
	Space(MyServer* pServer) :m_pServer(pServer) {}
	std::set<SpEntity> setEntity;
	void Update();
	MyServer * const m_pServer;
};