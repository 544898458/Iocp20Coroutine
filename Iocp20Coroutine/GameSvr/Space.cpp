#include "pch.h"
#include "Space.h"
#include "Entity.h"
#include "PlayerComponent.h"
#include "GameSvrSession.h"
#include "资源Component.h"
#include "PlayerComponent.h"
#include "AoiComponent.h"
#include "AttackComponent.h"

Space::Space(const std::string& stf寻路文件)
{
	std::shared_ptr<CrowdToolState> CreateCrowdToolState(const std::string & stf寻路文件);
	m_spCrowdToolState = CreateCrowdToolState(stf寻路文件);
}

Space::~Space()
{
	EraseEntity(true);
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

inline WpEntity Space::Get最近的Entity(Entity& refEntity, const bool bFindEnemy, std::function<bool(const Entity&)> fun符合条件)
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
			return refEntity.DistancePow2(*sp1) < refEntity.DistancePow2(*sp2);
		});
	return iterMin->second->weak_from_this();
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

	auto [iterNew, bOk] = g_mapSpace.insert({ idSpace,std::make_shared<Space>("all_tiles_tilecache.bin") });
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

		if (spEntity->m_spPlayer)
		{
			spEntity->m_spPlayer->m_refSession.Erase(spEntity->Id);
		}

		LOG(INFO) << "删除对象," << spEntity->NickName() << ",pEntity=" << spEntity << ",删除前剩余" << m_mapEntity.size();
		spEntity->OnDestroy();
		iter = m_mapEntity.erase(iter);
	}
}

std::tuple<int, int, int> 格子(const Position& refPos)
{
	const uint8_t u8格子正方形边长 = 10;
	const uint16_t u16X坐标放大倍数 = 10000;//z坐标不能超过9999
	const int32_t i32格子X = ((int)refPos.x) / u8格子正方形边长;
	const int32_t i32格子Z = ((int)refPos.z) / u8格子正方形边长;
	const int32_t i32格子ID = i32格子X * u16X坐标放大倍数 + i32格子Z;
	return { i32格子ID ,i32格子X, i32格子Z };
}


std::tuple<int, int, int> 格子(const Entity& refEntity)
{
	return 格子(refEntity.m_Pos);
}

std::vector<int> Space::GetEntity能看到的格子(const Entity& refEntity)
{
	std::vector<int> vec;
	const auto [i32格子Id, i32格子X, i32格子Z] = 格子(refEntity);
	if (!refEntity.m_upAoi)
		return {};

	int i32视野范围 = refEntity.m_upAoi->m_i32视野范围;
	if (refEntity.m_spAttack)
		i32视野范围 = std::max<int>(refEntity.m_spAttack->m_f警戒距离, i32视野范围);

	++i32视野范围;
	for (int x = i32格子X - i32视野范围; x < i32格子X + i32视野范围; ++x)
	{
		for (int z = i32格子Z - i32视野范围; z < i32格子Z + i32视野范围; ++z)
		{
			const auto [id, _, __] = 格子({ (float)x,(float)z });
			vec.push_back(id);
		}
	}

	return vec;
}

int Space::Get怪物单位数()
{
	return Get单位数([](const Entity& refEntity)
		{
			if (refEntity.IsDead())
				return false;

			if (nullptr == refEntity.m_spMonster)
				return false;

			if (refEntity.m_spPlayer)//属于玩家制的单位
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
			if (nullptr == refEntity.m_spPlayer)
				return false;

			if (&ref != &refEntity.m_spPlayer->m_refSession)
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

void Space::AddEntity(SpEntity& spNewEntity)
{
	m_mapEntity.insert({ spNewEntity->Id ,spNewEntity });//全地图单位
	AoiComponent::Add(*this, *spNewEntity);
}
