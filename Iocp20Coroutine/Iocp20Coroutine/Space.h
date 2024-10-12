#pragma once
#include <map>
#include <set>
#include "SpEntity.h"

//class GameSvr;
/// <summary>
/// Go”Ô—‘GoWorldøÚº‹Space/Entity£¨œ‡µ±”⁄C#”Ô—‘ETøÚº‹Scene/Unit
/// </summary>
class Space
{
public:
	Space() = default;
	Space(const Space&) = delete;
	template<class T>
	void Broadcast(const T& msg);

	std::map<int64_t, SpEntity> m_mapEntity;
	void Update();
	//GameSvr * const m_pServer;
};

//Space(GameSvr* pServer) = default;
class PlayerGateSession_Game;
template<class T>
void Space::Broadcast(const T& msg)
{
	std::set<PlayerGateSession_Game*> setEntity;
	for (const auto& [k, spEntity] : m_mapEntity)
	{
		assert(spEntity);
		
		if (!spEntity->m_spPlayer)
			continue;
		
		if (setEntity.find(&spEntity->m_spPlayer->m_refSession) != setEntity.end())
			continue;

		spEntity->m_spPlayer->m_refSession.Send(msg);
		setEntity.insert(&spEntity->m_spPlayer->m_refSession);
	}
}
