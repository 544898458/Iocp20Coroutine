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
/// Go语言GoWorld框架Space/Entity，相当于C#语言ET框架Scene/Unit
/// </summary>
class Space final
{
public:
	Space(const std::string& stf寻路文件);
	Space(const Space&) = delete;
	~Space();
	template<class T>
	void Broadcast(const T& msg);

	std::map<int64_t, SpEntity> m_mapEntity;
	WpEntity GetEntity(const int64_t id);
	
	//SpaceId：1无限刷怪
	static WpSpace AddSpace(const uint8_t idSpace);
	static WpSpace GetSpace(const uint8_t idSpace);
	static void StaticUpdate();
	void Update();
	bool CrowdTool可站立(const Position& refPos);
	bool CrowdToolFindNerestPos(Position& refPos);

	std::shared_ptr<CrowdToolState> m_spCrowdToolState;
	std::unordered_map<int, uint64_t> m_mapEntityId;
	int Get怪物单位数();
	int Get资源单位数(const 资源类型 类型);
	int Get玩家单位数(const PlayerGateSession_Game& ref);
	int Get单位数(const std::function<bool(const Entity&)>& fun是否统计此单位);
	/// <summary>
	/// 
	/// </summary>
	/// <param name="spNewEntity"></param>
	/// <param name="i32视野范围">如果是0就用警戒范围当成视野范围</param>
	void AddEntity(SpEntity& spNewEntity, const int32_t i32视野范围 = 0);
	WpEntity Get最近的Entity支持地堡中的单位(Entity& refEntity, const bool bFindEnemy, std::function<bool(const Entity&)> fun符合条件);
	std::unordered_map<int, std::map<uint64_t, WpEntity>> m_map能看到这一格;
	std::unordered_map<int, std::map<uint64_t, WpEntity>> m_map在这一格里;
private:
	void EraseEntity(const bool bForceEraseAll);
	WpEntity Get最近的Entity(Entity& refEntity, const bool bFindEnemy, std::function<bool(const Entity&)> fun符合条件);

};


class PlayerGateSession_Game;
