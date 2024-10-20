#include "pch.h"
#include "地堡Component.h"
#include "Entity.h"
#include "Space.h"

void 地堡Component::AddComponet(Entity& refEntity, PlayerGateSession_Game& refGateSession)
{
	refEntity.m_sp地堡 = std::make_shared<地堡Component>();
}

void 地堡Component::进(Space& refSpace, uint64_t idEntity)
{
	//从地图上删除，记录在地堡内
	auto wp = refSpace.GetEntity(idEntity);
	CHECK_RET_VOID(!wp.expired());
	auto sp = wp.lock();
	m_vecSpEntity.push_back(sp);
	refSpace.m_mapEntity.erase(idEntity);
	sp->BroadcastLeave();
}
