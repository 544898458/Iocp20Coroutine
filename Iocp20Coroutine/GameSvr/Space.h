#pragma once
#include <map>
#include <set>
#include <functional>
#include <algorithm>
#include "SpEntity.h"
#include "Entity.h"
#include "SpSpace.h"
class CrowdToolState;
struct 副本配置
{
	typedef CoTask<int>(*funCo副本剧情)(Space& refSpace, FunCancel& funCancel, const std::string strPlayerNickName);

	std::string str寻路文件名;
	std::string strSceneName;
	funCo副本剧情 funCo剧情;
	std::string strHttps音乐;
};

//class GameSvr;
/// <summary>
/// Go语言GoWorld框架Space/Entity，相当于C#语言ET框架Scene/Unit
/// </summary>
class Space final
{
public:
	Space(const 副本配置& ref配置);// std::string& stf寻路文件);
	Space(const Space&) = delete;
	~Space();
	void Save(uint8_t idSpace);
	void Load(uint8_t idSpace);
	template<class T>
	void Broadcast(const T& msg);

	std::map<int64_t, SpEntity> m_mapEntity;
	WpEntity GetEntity(const int64_t id);

	//SpaceId：1无限刷怪
	static WpSpace AddSpace(const uint8_t idSpace);
	static WpSpace GetSpace(const uint8_t idSpace);
	static void StaticOnAppExit();
	static void StaticUpdate();
	static std::tuple<bool, WpSpace> GetSpace单人(const std::string& refStrPlayerNickName, const 副本配置& 配置);
	static WpSpace GetSpace单人(const std::string& refStrPlayerNickName);
	static bool DeleteSpace单人(const std::string& refStrPlayerNickName);
	static const std::unordered_map<std::string, SpSpace>& 个人战局();
	void Update();
	bool CrowdTool可站立(const Position& refPos);
	bool CrowdToolFindNerestPos(Position& refPos);

	int Get怪物单位数(const 单位类型 类型= 单位类型_Invalid_0)const;
	int Get资源单位数(const 单位类型 类型);
	int Get玩家单位数(const std::string& strPlayerNickName);
	int Get单位数(const std::function<bool(const Entity&)>& fun是否统计此单位)const;
	int Get单位数(const 单位类型 arg类型)const;
	/// <summary>
	/// 
	/// </summary>
	/// <param name="spNewEntity"></param>
	/// <param name="i32视野范围">如果是0就用警戒范围当成视野范围</param>
	void AddEntity(SpEntity spNewEntity, const int32_t i32视野范围 = 0);
	void 所有玩家全退出();
	void OnDestory();
	enum FindType
	{
		所有,
		敌方,
		友方,
	};
	WpEntity Get最近的Entity支持地堡中的单位(Entity& refEntity, FindType bFindEnemy, std::function<bool(const Entity&)> fun符合条件);

	WpEntity Get最近的Entity(Entity& refEntity, FindType bFindEnemy, std::function<bool(const Entity&)> fun符合条件);
	WpEntity Get最近的Entity(Entity& refEntity, const FindType bFindEnemy, const 单位类型 目标类型);
	std::unordered_map<int, std::map<uint64_t, WpEntity>> m_map能看到这一格;
	std::unordered_map<int, std::map<uint64_t, WpEntity>> m_map在这一格里;

	std::unordered_map<uint64_t, WpEntity> m_map视口;
	struct SpacePlayer
	{
		SpacePlayer();
		SpacePlayer(const SpacePlayer&) = delete;
		/// <summary>
		/// 加入Space空间的实体（玩家角色）
		/// </summary>
		std::map<uint64_t, WpEntity> m_mapWpEntity;
		uint32_t m_u32燃气矿 = 10;
		uint32_t m_u32晶体矿 = 100;
		Msg剧情对话 m_msg上次发给前端的剧情对话;

		void OnDestroy(const bool b删除玩家所有单位, Space& refSpace, const std::string& refStrNickName);
		void Erase(uint64_t u64Id);
	};
	//using SpSpacePlayer = std::shared_ptr<SpacePlayer> ;
	std::unordered_map<std::string, SpacePlayer> m_mapPlayer;
	static SpacePlayer& GetSpacePlayer(const Entity& ref);
	Space::SpacePlayer& GetSpacePlayer(const std::string strPlayerNickName);
	WpEntity 造活动单位(Entity& ref视口, const std::string& refStrNickName, const 单位类型 类型, const Position& refPos, bool b设置视口 = false);
	WpEntity 造活动单位(std::shared_ptr<PlayerComponent>& refSpPlayer可能空, const std::string& refStrNickName, const Position& refPos, const 单位类型 类型);

	bool 可放置建筑(const Position& pos, float f半边长);
	const 副本配置 m_配置;
	FunCancel m_funCancel剧情;
	CrowdToolState& GetCrowdToolState(单位类型 类型);
	void On玩家离线(const std::string& refStrNickName离线);
	bool m_b休眠 = false;
private:
	void EraseEntity(const bool bForceEraseAll);
	std::shared_ptr<CrowdToolState> m_spCrowdToolState;
	std::shared_ptr<CrowdToolState> m_spCrowdToolState空中;
};


class PlayerGateSession_Game;
