#pragma once
#include <map>
#include <set>
#include <functional>
#include <algorithm>
#include "SpEntity.h"
#include "Entity.h"
#include "SpSpace.h"
class CrowdToolState;
//class GameSvr;
/// <summary>
/// Go����GoWorld���Space/Entity���൱��C#����ET���Scene/Unit
/// </summary>
class Space final
{
public:
	Space(const std::string& stfѰ·�ļ�);
	Space(const Space&) = delete;
	~Space();
	template<class T>
	void Broadcast(const T& msg);

	std::map<int64_t, SpEntity> m_mapEntity;
	WpEntity GetEntity(const int64_t id);
	WpEntity Get�����Entity(Entity& refEntity, const bool bFindEnemy, std::function<bool(const Entity&)> fun�������� )
	{
		std::vector<std::pair<int64_t, SpEntity>> vecEnemy;
		std::copy_if(m_mapEntity.begin(), m_mapEntity.end(), std::back_inserter(vecEnemy),
			[bFindEnemy, &refEntity, &fun��������](const auto& pair)
			{
				auto& sp = pair.second;
				CHECK_FALSE(sp);
				const auto bEnemy = sp->IsEnemy(refEntity);
				if (bEnemy != bFindEnemy)
					return false;

				if (fun�������� && !fun��������(*sp))
					return false;

				return sp.get() != &refEntity && !sp->IsDead();
			});

		if (vecEnemy.empty())
		{
			return {};
		}

		auto iterMin = std::min_element(vecEnemy.begin(), vecEnemy.end(), [&refEntity](const auto& pair1, const auto& pair2)
			{
				auto& sp1 = pair1.second;
				auto& sp2 = pair2.second;
				return refEntity.DistancePow2(*sp1) < refEntity.DistancePow2(*sp2);
			});
		return iterMin->second->weak_from_this();
	}
	//SpaceId��1����ˢ��
	static WpSpace AddSpace(const uint8_t idSpace);
	static WpSpace GetSpace(const uint8_t idSpace);
	static void StaticUpdate();
	void Update();
	bool CrowdTool��վ��(const Position& refPos);

	std::shared_ptr<CrowdToolState> m_spCrowdToolState;
	std::unordered_map<int, uint64_t> m_mapEntityId;
private:
	void EraseEntity(const bool bForceEraseAll);
	
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
