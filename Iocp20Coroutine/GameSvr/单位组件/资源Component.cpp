#include "pch.h"
#include "资源Component.h"
#include "Space.h"
#include "MyMsgQueue.h"
#include "Entity.h"
#include "临时阻挡Component.h"
#include "单位.h"

void 资源Component::Add(Space& refSpace, const 单位类型 类型, const Position& refPosition)
{
	单位::单位配置 配置 = {};
	if (!单位::Find单位配置(类型, 配置))
	{
		LOG(ERROR) << "Add" << 类型;
		_ASSERT(false);
		return;
	}

	SpEntity spEntity = std::make_shared<Entity, const Position&, Space&, const 单位类型, const 单位::单位配置&>(
		refPosition, refSpace, std::forward<const 单位类型&&>(类型), 配置);
	refSpace.AddEntity(spEntity);
	//LOG(INFO) << "SpawnMonster:" << refSpace.m_mapEntity.size();
	spEntity->m_up资源 = std::make_shared<资源Component, const 单位类型>(std::forward<const 单位类型&&>(类型));
	临时阻挡Component::AddComponent(*spEntity, 2.f);
	spEntity->BroadcastEnter();
}