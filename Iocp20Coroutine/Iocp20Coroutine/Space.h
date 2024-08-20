#pragma once
#include <map>
#include "SpEntity.h"

class GameSvr;
/// <summary>
/// Go”Ô—‘GoWorldøÚº‹Space/Entity£¨œ‡µ±”⁄C#”Ô—‘ETøÚº‹Scene/Unit
/// </summary>
class Space
{
public:
	Space(GameSvr* pServer) :m_pServer(pServer) {}
	std::map<int64_t,SpEntity> m_mapEntity;
	void Update();
	GameSvr * const m_pServer;
};