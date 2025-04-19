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
	typedef CoTask<int>(*funCo��������)(Space& refSpace, FunCancel& funCancel, const std::string strPlayerNickName);

	std::string strѰ·�ļ���;
	std::string strSceneName;
	funCo�������� funCo����;
	std::string strHttps����;
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
	static std::tuple<bool, WpSpace> GetSpace����(const std::string& refStrPlayerNickName, const ��������& ����);
	static WpSpace GetSpace����(const std::string& refStrPlayerNickName);
	static bool DeleteSpace����(const std::string& refStrPlayerNickName);
	static const std::unordered_map<std::string, SpSpace>& ����ս��();
	void Update();
	bool CrowdTool��վ��(const Position& refPos);
	bool CrowdToolFindNerestPos(Position& refPos);
	bool CrowdTool����ֱ��(const Position& pos��ʼ, const Position& posĿ��);
	int Get���ﵥλ��(const ��λ���� ���� = ��λ����_Invalid_0)const;
	int Get��Դ��λ��(const ��λ���� ����);
	int Get��ҵ�λ��(const std::string& strPlayerNickName, const ��λ���� ���� = ��λ����::��λ����_Invalid_0);
	int Get��λ��(const std::function<bool(const Entity&)>& fun�Ƿ�ͳ�ƴ˵�λ)const;
	int Get��λ��(const ��λ���� arg����)const;
	/// <summary>
	/// 
	/// </summary>
	/// <param name="spNewEntity"></param>
	/// <param name="i32��Ұ��Χ">�����0���þ��䷶Χ������Ұ��Χ</param>
	void AddEntity(SpEntity spNewEntity, const int32_t i32��Ұ��Χ = 0);
	void �������ȫ�˳�();
	void OnDestory();

	std::unordered_map<int, std::map<uint64_t, WpEntity>> m_map�ܿ�����һ��;
	std::unordered_map<int, std::map<uint64_t, WpEntity>> m_map����һ����;

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
		Msg����Ի� m_msg�ϴη���ǰ�˵ľ���Ի�;

		void OnDestroy(const bool bɾ��������е�λ, Space& refSpace, const std::string& refStrNickName);
		size_t Erase(uint64_t u64Id);

		bool ��ʼ������λ(const ��λ���� ����, Entity& refEntity����);
		bool �������(const ��λ���� ����, Entity& refEntity����);
		bool �ѽ���(const ��λ���� ����)const;

		bool ��ʼ������λ����(const ��λ���� ����, const �������� ����, Entity& refEntity�ڴ˽����н���);
		bool ������λ�������(const ��λ���� ��λ, const �������� ����, Entity& refEntity�ڴ˽����н���);
		uint16_t ��λ���Եȼ�(const ��λ���� ��λ, const �������� ����)const;
		

		struct ��λ����
		{
			bool b�ѽ��� = false;
			WpEntity wpEntity�ڴ˽����н���;
		};
		std::map<��λ����, ��λ����> m_map��λ����;

		struct ��λ_���Եȼ�
		{
			uint16_t u16�ȼ� = 0;
			WpEntity wpEntity�ڴ˽���������;
		};
		using MAP_��λ���Եȼ� = std::map<��λ����, std::map<��������, ��λ_���Եȼ� > >;
		MAP_��λ���Եȼ� m_map��λ���Եȼ�;
	};
	//using SpSpacePlayer = std::shared_ptr<SpacePlayer> ;
	std::unordered_map<std::string, SpacePlayer> m_mapPlayer;
	static SpacePlayer& GetSpacePlayer(const Entity& ref);
	Space::SpacePlayer& GetSpacePlayer(const std::string strPlayerNickName);
	WpEntity ����λ(Entity& ref�ӿ�, const std::string& refStrNickName, const ��λ���� ����, const Position& refPos, bool b�����ӿ� = false);
	WpEntity ����λ(std::shared_ptr<PlayerComponent>& refSpPlayer���ܿ�, const std::string& refStrNickName, const Position& refPos, const ��λ���� ����);

	bool �ɷ��ý���(const Position& pos, float f��߳�);
	const �������� m_����;
	FunCancel m_funCancel����;
	CrowdToolState& GetCrowdToolState(��λ���� ����);
	void On�������(const std::string& refStrNickName����);
	bool m_b���� = false;
private:
	void EraseEntity(const bool bForceEraseAll);
	std::shared_ptr<CrowdToolState> m_spCrowdToolState;
	std::shared_ptr<CrowdToolState> m_spCrowdToolState����;
};


class PlayerGateSession_Game;
