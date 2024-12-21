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
	
	//SpaceId��1����ˢ��
	static WpSpace AddSpace(const uint8_t idSpace);
	static WpSpace GetSpace(const uint8_t idSpace);
	static void StaticUpdate();
	void Update();
	bool CrowdTool��վ��(const Position& refPos);
	bool CrowdToolFindNerestPos(Position& refPos);

	std::shared_ptr<CrowdToolState> m_spCrowdToolState;
	std::unordered_map<int, uint64_t> m_mapEntityId;
	int Get���ﵥλ��();
	int Get��Դ��λ��(const ��Դ���� ����);
	int Get��ҵ�λ��(const PlayerGateSession_Game& ref);
	int Get��λ��(const std::function<bool(const Entity&)>& fun�Ƿ�ͳ�ƴ˵�λ);
	/// <summary>
	/// 
	/// </summary>
	/// <param name="spNewEntity"></param>
	/// <param name="i32��Ұ��Χ">�����0���þ��䷶Χ������Ұ��Χ</param>
	void AddEntity(SpEntity& spNewEntity, const int32_t i32��Ұ��Χ = 0);
	WpEntity Get�����Entity֧�ֵر��еĵ�λ(Entity& refEntity, const bool bFindEnemy, std::function<bool(const Entity&)> fun��������);
	std::unordered_map<int, std::map<uint64_t, WpEntity>> m_map�ܿ�����һ��;
	std::unordered_map<int, std::map<uint64_t, WpEntity>> m_map����һ����;
private:
	void EraseEntity(const bool bForceEraseAll);
	WpEntity Get�����Entity(Entity& refEntity, const bool bFindEnemy, std::function<bool(const Entity&)> fun��������);

};


class PlayerGateSession_Game;
