#include "pch.h"
#include "地堡Component.h"
#include "AttackComponent.h"
#include "Entity.h"
#include "Space.h"
#include "EntitySystem.h"
#include "PlayerComponent.h"
#include "AoiComponent.h"
#include "PlayerGateSession_Game.h"

void 地堡Component::AddComponent(Entity& refEntity)
{
	CHECK_RET_VOID(!refEntity.m_up地堡);
	refEntity.m_up地堡.reset(new 地堡Component(refEntity));
}


void 可进活动单位Component::AddComponent(Entity& refEntity)
{
    refEntity.AddComponentOnDestroy(&Entity::m_up可进活动单位, refEntity);
}

void 可进活动单位Component::OnEntityDestroy(const bool bDestroy)
{
	for (auto& sp : m_listSpEntity)
		sp->OnDestroy();

	m_listSpEntity.clear();
}

void 可进活动单位Component::OnBeforeDelayDelete()
{
	全都出去();
}

void 可进活动单位Component::进(Space& refSpace, Entity& refEntity)
{
	if (m_refEntity.IsDead())
	{
		PlayerComponent::Say(m_refEntity, "地堡已摧毁不可再进入", SayChannel::系统);
		return;
	}
	if (m_listSpEntity.size() >= 10)
	{
		PlayerComponent::Say(m_refEntity, "地堡人满了", SayChannel::系统);
		return;
	}

	//从地图上删除，记录在地堡内
	if (refEntity.IsDead())
	{
		LOG(INFO) << "阵亡单位不能进地堡";//_ASSERTfalse);
		return;
	}
	refEntity.SetPos(m_refEntity.Pos());
	refEntity.BroadcastLeave();//OnDestroy();
	if (refEntity.m_upAoi)
		refEntity.m_upAoi->离开Space();

	m_listSpEntity.push_back(refEntity.shared_from_this());
	refSpace.m_mapEntity.erase(refEntity.Id);
	
	refEntity.m_wpOwner = m_refEntity.weak_from_this();

	if (refEntity.m_upPlayer)
		refEntity.m_upPlayer->m_refSession.删除选中(refEntity.Id);

	EntitySystem::BroadcastEntity描述(m_refEntity, std::format("地堡内有{0}人", m_listSpEntity.size()));
}

void 可进活动单位Component::全都出去()
{
	auto list = m_listSpEntity;
	m_listSpEntity.clear();
	for (auto& sp : list)
	{
		//m_refEntity.m_refSpace.m_mapEntity.insert({ sp->Id, sp });
		m_refEntity.m_refSpace.AddEntity(sp);
		auto pos = sp->Pos();
		m_refEntity.m_refSpace.CrowdToolFindNerestPos(pos);
		sp->SetPos(pos);
		sp->BroadcastEnter();
		sp->m_wpOwner.reset();
	}
	list.clear();

	EntitySystem::BroadcastEntity描述(m_refEntity, std::format("地堡是空的", m_listSpEntity.size()));
}