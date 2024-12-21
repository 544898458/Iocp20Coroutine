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

void Space::AddEntity(SpEntity& spNewEntity, const int32_t i32视野范围)
{
	m_mapEntity.insert({ spNewEntity->Id ,spNewEntity });//全地图单位
	AoiComponent::Add(*this, *spNewEntity,i32视野范围);
}
