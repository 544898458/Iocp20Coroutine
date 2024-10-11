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
	//Space(GameSvr* pServer) = default;
	template<class T>
	void Broadcast(const T& msg);

	std::map<int64_t, SpEntity> m_mapEntity;
	void Update();
	//GameSvr * const m_pServer;
};

//Space(GameSvr* pServer) = default;

template<class T>
void Space::Broadcast(const T& msg)
{
	std::set<Entity*> setEntity;
	for (const auto& [k, spEntity] : m_mapEntity)
	{
		assert(spEntity);
		
		if (!spEntity->m_spPlayer)
			continue;
		
		if (setEntity.find(spEntity.get()) != setEntity.end())
			continue;

		spEntity->m_spPlayer->m_refSession.Send(msg);
		setEntity.insert(spEntity.get());
	}
}
