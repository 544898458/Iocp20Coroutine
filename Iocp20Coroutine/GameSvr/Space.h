#pragma once
#include <map>
#include <set>
#include <functional>
#include <algorithm>
#include "SpEntity.h"
#include "Entity.h"
#include "SpSpace.h"
class CrowdToolState;
struct ��������
{
	typedef CoTask<int>(*funCo��������)(Space& refSpace, Entity& ref�ӿ�, FunCancel& funCancel, PlayerGateSession_Game& refGateSession);

	std::string strѰ·�ļ���;
	std::string strSceneName;
	funCo�������� funCo����;
};

//class GameSvr;
/// <summary>
/// Go����GoWorld���Space/Entity���൱��C#����ET���Scene/Unit
/// </summary>
class Space final
{
public:
	Space(const ��������& ref����);// std::string& stfѰ·�ļ�);
	Space(const Space&) = delete;
	~Space();
	void Save(uint8_t idSpace);
	void Load(uint8_t idSpace);
	template<class T>
	void Broadcast(const T& msg);

	std::map<int64_t, SpEntity> m_mapEntity;
	WpEntity GetEntity(const int64_t id);

	//SpaceId��1����ˢ��
	static WpSpace AddSpace(const uint8_t idSpace);
	static WpSpace GetSpace(const uint8_t idSpace);
	static void StaticOnAppExit();
	static void StaticUpdate();
	void Update();
	bool CrowdTool��վ��(const Position& refPos);
	bool CrowdToolFindNerestPos(Position& refPos);

	std::shared_ptr<CrowdToolState> m_spCrowdToolState;
	std::unordered_map<int, uint64_t> m_mapEntityId;
	int Get���ﵥλ��();
	int Get��Դ��λ��(const ��λ���� ����);
	int Get��ҵ�λ��(const PlayerGateSession_Game& ref);
	int Get��λ��(const std::function<bool(const Entity&)>& fun�Ƿ�ͳ�ƴ˵�λ);
	/// <summary>
	/// 
	/// </summary>
	/// <param name="spNewEntity"></param>
	/// <param name="i32��Ұ��Χ">�����0���þ��䷶Χ������Ұ��Χ</param>
	void AddEntity(SpEntity spNewEntity, const int32_t i32��Ұ��Χ = 0);
	void �������ȫ�˳�();
	void OnDestory();
	enum FindType
	{
		����,
		�з�,
		�ѷ�,
	};	
	WpEntity Get�����Entity֧�ֵر��еĵ�λ(Entity& refEntity, FindType bFindEnemy, std::function<bool(const Entity&)> fun��������);

	WpEntity Get�����Entity(Entity& refEntity, FindType bFindEnemy, std::function<bool(const Entity&)> fun��������);
	std::unordered_map<int, std::map<uint64_t, WpEntity>> m_map�ܿ�����һ��;
	std::unordered_map<int, std::map<uint64_t, WpEntity>> m_map����һ����;

	std::unordered_map<std::string, std::map<uint64_t, WpEntity>> m_map������PlayerEntity;
	std::unordered_map<uint64_t, WpEntity> m_map�ӿ�;
	struct SpacePlayer
	{
		SpacePlayer();
		SpacePlayer(const SpacePlayer&) = delete;
		/// <summary>
		/// ����Space�ռ��ʵ�壨��ҽ�ɫ��
		/// </summary>
		std::map<uint64_t, WpEntity> m_mapWpEntity;
		uint32_t m_u32ȼ���� = 10;
		uint32_t m_u32����� = 100;

		void OnDestroy(const bool b���˸���, Space& refSpace, const std::string& refStrNickName);
		void Erase(uint64_t u64Id);
	};
	//using SpSpacePlayer = std::shared_ptr<SpacePlayer> ;
	std::unordered_map<std::string, SpacePlayer> m_mapPlayer;
	static SpacePlayer& GetSpacePlayer(const Entity& ref);
	SpEntity ����λ(std::shared_ptr<PlayerComponent>& refSpPlayer, const std::string& strNickName, const Position& pos, const ��λ::���λ����& ����, const ��λ���� ����);

	bool �ɷ��ý���(const Position& pos, float f��߳�);
	const �������� m_����;
private:
	void EraseEntity(const bool bForceEraseAll);


};


class PlayerGateSession_Game;
