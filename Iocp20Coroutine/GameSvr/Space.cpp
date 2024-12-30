#include "pch.h"
#include "Space.h"
#include "Entity.h"
#include "PlayerComponent.h"
#include "GameSvrSession.h"
#include "资源Component.h"
#include "PlayerComponent.h"
#include "AoiComponent.h"
#include "AttackComponent.h"
#include "PlayerNickNameComponent.h"
#include "EntitySystem.h"
#include "DefenceComponent.h"
#include "走Component.h"
#include "采集Component.h"
#include "造建筑Component.h"

Space::Space(const 副本配置& ref) :m_配置(ref)
{
	std::shared_ptr<CrowdToolState> CreateCrowdToolState(const std::string & stf寻路文件);
	m_spCrowdToolState = CreateCrowdToolState(ref.str寻路文件名);
}

Space::~Space()
{
}

WpEntity Space::GetEntity(const int64_t id)
{
	auto itFind = m_mapEntity.find(id);
	if (itFind == m_mapEntity.end())
	{
		LOG(INFO) << "选中的实体不存在:" << id;
		//assert(false);
		return {};
	}
	CHECK_DEFAULT(itFind->second);
	return itFind->second->weak_from_this();
}

WpEntity Space::Get最近的Entity支持地堡中的单位(Entity& refEntity, const bool bFindEnemy, std::function<bool(const Entity&)> fun符合条件)
{
	WpEntity wp = refEntity.m_wpOwner;
	if (!GetEntity(refEntity.Id).expired())
	{
		wp = refEntity.weak_from_this();
	}
	if (wp.expired())
		return {};

	return Get最近的Entity(*wp.lock(), bFindEnemy, fun符合条件);
}

WpEntity Space::Get最近的Entity(Entity& refEntity, const bool bFindEnemy, std::function<bool(const Entity&)> fun符合条件)
{
	if (!refEntity.m_upAoi)
		return{};

	std::vector<std::pair<int64_t, WpEntity>> vecEnemy;
	std::copy_if(refEntity.m_upAoi->m_map我能看到的.begin(), refEntity.m_upAoi->m_map我能看到的.end(), std::back_inserter(vecEnemy),
		[bFindEnemy, &refEntity, &fun符合条件](const auto& pair)
		{
			auto& wp = pair.second;
			CHECK_FALSE(!wp.expired());
			Entity& ref = *wp.lock();
			const auto bEnemy = ref.IsEnemy(refEntity);
			if (bEnemy != bFindEnemy)
				return false;

			if (fun符合条件 && !fun符合条件(ref))
				return false;

			return &ref != &refEntity && !ref.IsDead();
		});

	if (vecEnemy.empty())
	{
		return {};
	}

	auto iterMin = std::min_element(vecEnemy.begin(), vecEnemy.end(), [&refEntity](const auto& pair1, const auto& pair2)
		{
			auto& sp1 = pair1.second;
			auto& sp2 = pair2.second;
			return refEntity.DistancePow2(*sp1.lock()) < refEntity.DistancePow2(*sp2.lock());
		});
	return iterMin->second.lock()->weak_from_this();
}

void Space::Update()
{
	EraseEntity(false);

	void CrowToolUpdate(Space & ref);
	CrowToolUpdate(*this);
}

std::unordered_map<uint8_t, SpSpace> g_mapSpace;
WpSpace Space::AddSpace(const uint8_t idSpace)
{
	auto wpOld = GetSpace(idSpace);
	if (!wpOld.expired())
		return wpOld;

	副本配置 配置;
	{
		bool Get副本配置(const 副本ID id, 副本配置 & refOut);
		const auto ok = Get副本配置(多人联机地图, 配置);
		CHECK_RET_DEFAULT(ok);
	}
	auto [iterNew, bOk] = g_mapSpace.insert({ idSpace,std::make_shared<Space,const 副本配置&>(配置) });
	assert(bOk);
	return iterNew->second;
}

WpSpace Space::GetSpace(const uint8_t idSpace)
{
	auto iterFind = g_mapSpace.find(idSpace);
	if (g_mapSpace.end() == iterFind)
		return {};

	return iterFind->second;
}

void Space::StaticUpdate()
{
	for (auto [id, sp] : g_mapSpace)
	{
		sp->Update();
	}
}

bool Space::CrowdTool可站立(const Position& refPos)
{
	if (!m_spCrowdToolState)
	{
		LOG(ERROR) << "m_spCrowdToolState";
		return true;
	}
	bool CrowdTool可站立(CrowdToolState & refCrowTool, const Position & refPos);
	return CrowdTool可站立(*m_spCrowdToolState, refPos);
}

bool Space::CrowdToolFindNerestPos(Position& refPos)
{
	if (!m_spCrowdToolState)
	{
		LOG(ERROR) << "m_spCrowdToolState";
		return false;
	}
	bool CrowdToolFindNerestPos(CrowdToolState & refCrowTool, Position & refPos);
	return CrowdToolFindNerestPos(*m_spCrowdToolState, refPos);
}

Space::SpacePlayer& Space::GetSpacePlayer(const Entity& ref)
{
	if (ref.m_spPlayerNickName)
		return ref.m_refSpace.m_mapPlayer[ref.m_spPlayerNickName->m_strNickName];

	LOG(ERROR) << ref.Id << ",不是玩家";
	assert(false);
	static Space::SpacePlayer s_err;
	return s_err;
}

void Space::EraseEntity(const bool bForceEraseAll)
{
	for (auto iter = m_mapEntity.begin(); iter != m_mapEntity.end(); )
	{
		auto& spEntity = iter->second;
		spEntity->Update();

		if (!bForceEraseAll && !spEntity->NeedDelete())
		{
			++iter;
			continue;
		}

		if (spEntity->m_spPlayerNickName)
		{
			m_mapPlayer[spEntity->m_spPlayerNickName->m_strNickName].Erase(spEntity->Id);
		}

		LOG(INFO) << "删除过期对象," << spEntity->NickName() << ",Id=" << spEntity->Id << ",删除前剩余" << m_mapEntity.size();
		spEntity->OnDestroy();
		iter = m_mapEntity.erase(iter);
	}
}

int Space::Get怪物单位数()
{
	return Get单位数([](const Entity& refEntity)
		{
			if (refEntity.IsDead())
				return false;

			if (nullptr == refEntity.m_spMonster)
				return false;

			if (refEntity.m_spPlayerNickName)//属于玩家制的单位
				return false;

			return true;
		});
}

int Space::Get资源单位数(const 资源类型 类型)
{
	return Get单位数([类型](const Entity& refEntity)
		{
			if (nullptr == refEntity.m_sp资源)
				return false;

			return 类型 == refEntity.m_sp资源->m_类型;
		});
}

int Space::Get玩家单位数(const PlayerGateSession_Game& ref)
{
	return Get单位数([&ref](const Entity& refEntity)
		{
			if (nullptr == refEntity.m_spPlayerNickName)
				return false;

			if (ref.NickName() != refEntity.m_spPlayerNickName->m_strNickName)
				return false;

			if (!refEntity.m_spDefence)
				return false;//视口

			return true;
		});
}
int Space::Get单位数(const std::function<bool(const Entity&)>& fun是否统计此单位)
{
	int i32单位数(0);
	for (const auto [k, spEntity] : m_mapEntity)
	{
		if (fun是否统计此单位 && !fun是否统计此单位(*spEntity))
			continue;

		++i32单位数;
	}

	return i32单位数;
}

void Space::AddEntity(SpEntity& spNewEntity, const int32_t i32视野范围)
{
	m_mapEntity.insert({ spNewEntity->Id ,spNewEntity });//全地图单位
	AoiComponent::Add(*this, *spNewEntity, i32视野范围);
}

void Space::所有玩家全退出()
{
	auto map = m_map视口;
	for (auto [id, wp] : map)//迭代过程会删除	m_map视口	项
	{
		CHECK_WP_CONTINUE(wp);
		auto& ref视口 = *wp.lock();
		CHECK_RET_VOID(EntitySystem::Is视口(ref视口));
		CHECK_RET_VOID(ref视口.m_spPlayer);
		ref视口.m_spPlayer->m_refSession.OnDestroy();//旁观的人退出
	}
}

void Space::OnDestory()
{
	所有玩家全退出();
	EraseEntity(true);
}

Space::SpacePlayer::SpacePlayer()
{
}

void Space::SpacePlayer::OnDestroy(const bool b单人副本, Space& refSpace, const std::string& refStrNickName)
{
	for (auto [_, wp] : m_mapWpEntity)
	{
		//assert(!wp.expired());
		if (wp.expired())
		{
			LOG(ERROR) << "删了单位，但是这里没删";
			continue;
		}
		auto sp = wp.lock();
		if (b单人副本 || EntitySystem::Is视口(*sp))
		{
			if (sp->m_refSpace.GetEntity(sp->Id).expired())
			{
				LOG(INFO) << "可能是地堡里的兵" << sp->NickName();
				continue;
			}
			LOG(INFO) << "m_mapEntity.size=" << sp->m_refSpace.m_mapEntity.size();
			sp->OnDestroy();
			auto countErase = sp->m_refSpace.m_mapEntity.erase(sp->Id);
			assert(1 == countErase);
		}
		else
		{
			sp->m_spPlayer.reset();
			refSpace.m_map已离线PlayerEntity[refStrNickName].insert({ sp->Id,sp });
		}
	}

	m_mapWpEntity.clear();
}

inline void Space::SpacePlayer::Erase(uint64_t u64Id)
{
	if (!m_mapWpEntity.contains(u64Id))
	{
		LOG(WARNING) << "ERR";
		return;
	}

	m_mapWpEntity.erase(u64Id);
}


SpEntity Space::造活动单位(std::shared_ptr<PlayerComponent> &refSpPlayer可能空, const std::string &strNickName, const Position& pos, const 单位::活动单位配置& 配置, const 活动单位类型 类型)
{
	SpEntity spNewEntity = std::make_shared<Entity, const Position&, Space&, const 单位::单位配置&>(
		pos, *this, 配置.配置);
	PlayerComponent::AddComponent(*spNewEntity, refSpPlayer可能空, strNickName);
	AttackComponent::AddComponent(*spNewEntity, 类型, 配置.战斗);
	DefenceComponent::AddComponent(*spNewEntity, 配置.制造.u16初始Hp);
	走Component::AddComponent(*spNewEntity);
	m_mapPlayer[strNickName].m_mapWpEntity[spNewEntity->Id] = spNewEntity;//自己控制的单位
	AddEntity(spNewEntity);//全地图单位
	spNewEntity->m_速度每帧移动距离 = 配置.战斗.f每帧移动距离;
	switch (类型)
	{
	case 工程车:
		采集Component::AddComponent(*spNewEntity);
		造建筑Component::AddComponent(*spNewEntity, 类型);
		PlayerComponent::播放声音(*spNewEntity, "语音/工程车准备就绪女声可爱版"); //refGateSession.Say语音提示("工程车可以开工了!");//SCV, good to go, sir. SCV可以开工了
		break;
	case 兵:PlayerComponent::播放声音(*spNewEntity, "语音/准备战斗男声正经版"); break;//refGateSession.Say语音提示("听说有人要买我的狗头？");//You want a piece of me, boy?想要我的一部分吗，小子？
	case 近战兵:PlayerComponent::播放声音(*spNewEntity, "tfbRdy00"); break;//refGateSession.Say语音提示("听说有人要我的斧头？");//You want a piece of me, boy?想要我的一部分吗，小子？
	case 三色坦克:PlayerComponent::播放声音(*spNewEntity, "语音/ttardy00"); break;
	default:break;
	}

	spNewEntity->BroadcastEnter();
	PlayerComponent::Send资源(*spNewEntity);

	return spNewEntity;
}


bool Space::可放置建筑(const Position& refPos, float f半边长)
{
	if (!CrowdTool可站立({ refPos.x - f半边长 ,refPos.z + f半边长 }))return false;
	if (!CrowdTool可站立({ refPos.x - f半边长 ,refPos.z - f半边长 }))return false;
	if (!CrowdTool可站立({ refPos.x + f半边长 ,refPos.z + f半边长 }))return false;
	if (!CrowdTool可站立({ refPos.x + f半边长 ,refPos.z - f半边长 }))return false;

	//遍历全地图所有建筑判断重叠
	for (const auto& kv : m_mapEntity)
	{
		auto& refEntity = *kv.second;
		const auto& refPosOld = refEntity.Pos();
		bool CrowdTool判断单位重叠(const Position & refPosOld, const Position & refPosNew, const float f半边长);
		if (CrowdTool判断单位重叠(refPos, refPosOld, f半边长))
			return false;
	}

	return true;
}