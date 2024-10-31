#include "pch.h"
#include "Space.h"
#include "Entity.h"
#include "PlayerComponent.h"
#include "GameSvrSession.h"

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

void Space::Update()
{
	EraseEntity(false);
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
			spEntity->m_spPlayer->m_refSession.Erase(spEntity);
		}

		LOG(INFO) << "ɾ������," << spEntity->NickName() << ",pEntity=" << spEntity << ",ɾ��ǰʣ��" << m_mapEntity.size();
		spEntity->OnDestroy();
		iter = m_mapEntity.erase(iter);
	}
}
