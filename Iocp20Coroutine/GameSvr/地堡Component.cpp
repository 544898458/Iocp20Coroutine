#include "pch.h"
#include "地堡Component.h"
#include "AttackComponent.h"
#include "Entity.h"
#include "Space.h"
#include "EntitySystem.h"
#include "PlayerComponent.h"

void 地堡Component::AddComponet(Entity& refEntity, PlayerGateSession_Game& refGateSession)
{
	refEntity.m_sp地堡 = std::make_shared<地堡Component,Entity&>(refEntity);
}

void 地堡Component::OnDestroy()
{
	m_listSpEntity.clear();
}

void 地堡Component::OnBeforeDelayDelete()
{
	全都出地堡();
}

void 地堡Component::进(Space& refSpace, uint64_t idEntity)
{
	if (m_listSpEntity.size() >= 10)
	{
		PlayerComponent::Say(m_refEntity, "地堡人满了", SayChannel::系统);
		return;
	}

	//从地图上删除，记录在地堡内
	auto wp = refSpace.GetEntity(idEntity);
	CHECK_RET_VOID(!wp.expired());
	auto sp = wp.lock();
	if (sp->IsDead())
	{
		LOG(INFO) << "阵亡单位不能进地堡";//assert(false);
		return;
	}
	sp->BroadcastLeave();//OnDestroy();
	sp->m_wpOwner = sp->weak_from_this();
	m_listSpEntity.push_back(sp);
	refSpace.m_mapEntity.erase(idEntity);
	//sp->BroadcastLeave();
	EntitySystem::BroadcastEntity描述(m_refEntity, std::format("地堡内有{0}人", m_listSpEntity.size()));
}

void 地堡Component::Update()
{
	for (auto& sp : m_listSpEntity) 
	{
		if (sp->m_spAttack)
			sp->m_spAttack->Update();
	}
}

void 地堡Component::全都出地堡()
{
	auto list = m_listSpEntity;
	m_listSpEntity.clear();
	for (auto& sp : list)
	{
		m_refEntity.m_refSpace.m_mapEntity.insert({ sp->Id, sp });
		sp->BroadcastEnter();
		sp->m_wpOwner.reset();
	}
	list.clear();
}