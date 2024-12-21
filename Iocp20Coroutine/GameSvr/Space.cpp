#include "pch.h"
#include "Space.h"
#include "Entity.h"
#include "PlayerComponent.h"
#include "GameSvrSession.h"
#include "��ԴComponent.h"
#include "PlayerComponent.h"
#include "AoiComponent.h"
#include "AttackComponent.h"

Space::Space(const std::string& stfѰ·�ļ�)
{
	std::shared_ptr<CrowdToolState> CreateCrowdToolState(const std::string & stfѰ·�ļ�);
	m_spCrowdToolState = CreateCrowdToolState(stfѰ·�ļ�);
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
		LOG(INFO) << "ѡ�е�ʵ�岻����:" << id;
		//assert(false);
		return {};
	}
	CHECK_DEFAULT(itFind->second);
	return itFind->second->weak_from_this();
}

WpEntity Space::Get�����Entity֧�ֵر��еĵ�λ(Entity& refEntity, const bool bFindEnemy, std::function<bool(const Entity&)> fun��������)
{
	WpEntity wp = refEntity.m_wpOwner;
	if (!GetEntity(refEntity.Id).expired())
	{
		wp = refEntity.weak_from_this();
	}
	if (wp.expired())
		return {};

	return Get�����Entity(*wp.lock(), bFindEnemy, fun��������);
}

WpEntity Space::Get�����Entity(Entity& refEntity, const bool bFindEnemy, std::function<bool(const Entity&)> fun��������)
{
	if (!refEntity.m_upAoi)
		return{};

	std::vector<std::pair<int64_t, WpEntity>> vecEnemy;
	std::copy_if(refEntity.m_upAoi->m_map���ܿ�����.begin(), refEntity.m_upAoi->m_map���ܿ�����.end(), std::back_inserter(vecEnemy),
		[bFindEnemy, &refEntity, &fun��������](const auto& pair)
		{
			auto& wp = pair.second;
			CHECK_FALSE(!wp.expired());
			Entity& ref = *wp.lock();
			const auto bEnemy = ref.IsEnemy(refEntity);
			if (bEnemy != bFindEnemy)
				return false;

			if (fun�������� && !fun��������(ref))
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

bool Space::CrowdTool��վ��(const Position& refPos)
{
	if (!m_spCrowdToolState)
	{
		LOG(ERROR) << "m_spCrowdToolState";
		return true;
	}
	bool CrowdTool��վ��(CrowdToolState & refCrowTool, const Position & refPos);
	return CrowdTool��վ��(*m_spCrowdToolState, refPos);
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

		LOG(INFO) << "ɾ������," << spEntity->NickName() << ",pEntity=" << spEntity << ",ɾ��ǰʣ��" << m_mapEntity.size();
		spEntity->OnDestroy();
		iter = m_mapEntity.erase(iter);
	}
}

int Space::Get���ﵥλ��()
{
	return Get��λ��([](const Entity& refEntity)
		{
			if (refEntity.IsDead())
				return false;

			if (nullptr == refEntity.m_spMonster)
				return false;

			if (refEntity.m_spPlayer)//��������Ƶĵ�λ
				return false;

			return true;
		});
}

int Space::Get��Դ��λ��(const ��Դ���� ����)
{
	return Get��λ��([����](const Entity& refEntity)
		{
			if (nullptr == refEntity.m_sp��Դ)
				return false;

			return ���� == refEntity.m_sp��Դ->m_����;
		});
}

int Space::Get��ҵ�λ��(const PlayerGateSession_Game& ref)
{
	return Get��λ��([&ref](const Entity& refEntity)
		{
			if (nullptr == refEntity.m_spPlayer)
				return false;

			if (&ref != &refEntity.m_spPlayer->m_refSession)
				return false;

			if (!refEntity.m_spDefence)
				return false;//�ӿ�

			return true;
		});
}
int Space::Get��λ��(const std::function<bool(const Entity&)>& fun�Ƿ�ͳ�ƴ˵�λ)
{
	int i32��λ��(0);
	for (const auto [k, spEntity] : m_mapEntity)
	{
		if (fun�Ƿ�ͳ�ƴ˵�λ && !fun�Ƿ�ͳ�ƴ˵�λ(*spEntity))
			continue;

		++i32��λ��;
	}

	return i32��λ��;
}

void Space::AddEntity(SpEntity& spNewEntity, const int32_t i32��Ұ��Χ)
{
	m_mapEntity.insert({ spNewEntity->Id ,spNewEntity });//ȫ��ͼ��λ
	AoiComponent::Add(*this, *spNewEntity,i32��Ұ��Χ);
}
