#include "pch.h"
#include "可进活动单位Component.h"
#include "走Component.h"
#include "AttackComponent.h"
#include "找目标走过去Component.h"
#include "数值Component.h"
#include "Entity.h"
#include "Space.h"
#include "EntitySystem.h"
#include "PlayerComponent.h"
#include "AoiComponent.h"
#include "PlayerGateSession_Game.h"
#include "../../CoRoutine/CoEvent.h"
#include "../MyEvent.h"
#include "../枚举/属性类型.h"

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
	if (房虫 == m_refEntity.m_类型) 
	{
		for (auto& sp : m_listSpEntity)
		{
			数值Component::Set(*sp, 生命, 1);
		}
	}
	全都出去();
}

void 可进活动单位Component::进(Space& refSpace, Entity& refEntity)
{
	if (m_refEntity.IsDead())
	{
		PlayerComponent::Say(m_refEntity, "不可再进入", SayChannel::系统);
		return;
	}
	if (m_listSpEntity.size() >= 10)
	{
		PlayerComponent::Say(m_refEntity, "人满了", SayChannel::系统);
		return;
	}

	//从地图上删除，记录在地堡内
	if (refEntity.IsDead())
	{
		LOG(INFO) << "阵亡单位不能进地堡";//_ASSERTfalse);
		return;
	}

	if (refEntity.m_up找目标走过去)
	{
		refEntity.m_up找目标走过去->m_fun空闲走向此处 = nullptr;
		refEntity.m_up找目标走过去->m_b原地坚守 = false;
	}
	refEntity.SetPos(m_refEntity.Pos());
	refEntity.BroadcastLeave();//OnDestroy();
	if (refEntity.m_upAoi)
		refEntity.m_upAoi->离开Space();

	m_listSpEntity.push_back(refEntity.shared_from_this());
	refSpace.m_mapEntity.erase(refEntity.Id);
	if (refEntity.m_up走 && 0 <= refEntity.m_up走->m_idxCrowdAgent)
	{
		LOG(ERROR) << "" << refEntity.Id;
		_ASSERT(false);
	}
	refEntity.m_wpOwner = m_refEntity.weak_from_this();

	if (refEntity.m_upPlayer)
		refEntity.m_upPlayer->m_refSession.删除选中(refEntity.Id);

	EntitySystem::BroadcastEntity描述(m_refEntity, std::format("内有{0}人", m_listSpEntity.size()));
	CoEvent<MyEvent::进活动单位>::OnRecvEvent({ m_refEntity.shared_from_this() });
}

void 可进活动单位Component::全都出去()
{
	auto list = m_listSpEntity;
	m_listSpEntity.clear();
	for (auto& sp : list)
	{
		//m_refEntity.m_refSpace.m_mapEntity.insert({ sp->Id, sp });
		m_refEntity.m_refSpace.AddEntity(sp);
		auto pos = m_refEntity.Pos();
		m_refEntity.m_refSpace.CrowdToolFindNerestPos(pos);
		sp->SetPos(pos);
		sp->BroadcastEnter();
		sp->m_wpOwner.reset();
	}
	list.clear();

	EntitySystem::BroadcastEntity描述(m_refEntity, std::format("内有0人", m_listSpEntity.size()));
}