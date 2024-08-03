#include "StdAfx.h"
#include "Space.h"
#include "Entity.h"
#include "PlayerComponent.h"
#include "MySession.h"

void Space::Update()
{
	for (auto iter = setEntity.begin() ; iter!= setEntity.end() ; )
	{
		auto spEntity = *iter;
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

		LOG(INFO) << "É¾³ý¶ÔÏó,pEntity=" << spEntity;
		iter = setEntity.erase(iter);
	}
}
