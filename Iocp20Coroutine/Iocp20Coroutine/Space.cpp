#include "StdAfx.h"
#include "Space.h"
#include "Entity.h"
#include "PlayerComponent.h"
#include "GameSvrSession.h"

void Space::Update()
{
	for (auto iter = m_mapEntity.begin(); iter != m_mapEntity.end(); )
	{
		auto &spEntity = iter->second;
		spEntity->Update();

		if (!spEntity->NeedDelete())
		{
			++iter;
			continue;
		}

		spEntity->Broadcast(MsgDelRoleRet((uint64_t)spEntity.get()));

		if (spEntity->m_spPlayer)
		{
			spEntity->m_spPlayer->m_pSession->Erase(spEntity);
		}

		LOG(INFO) << "É¾³ý¶ÔÏó," << spEntity->NickName() << ",pEntity=" << spEntity << ",É¾³ýÇ°Ê£Óà" << m_mapEntity.size();
		iter = m_mapEntity.erase(iter);
	}
}
