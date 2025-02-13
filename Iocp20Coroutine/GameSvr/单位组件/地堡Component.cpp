#include "pch.h"
#include "地堡Component.h"
#include "AttackComponent.h"
#include "Entity.h"
#include "Space.h"
#include "EntitySystem.h"
#include "PlayerComponent.h"
#include "AoiComponent.h"
#include "PlayerGateSession_Game.h"

void 地堡Component::AddComponet(Entity& refEntity)
{
	refEntity.m_sp地堡 = std::make_shared<地堡Component,Entity&>(refEntity);
}

void 地堡Component::OnDestroy()
{
	for (auto& sp : m_listSpEntity)
		sp->OnDestroy();

	m_listSpEntity.clear();
}

void 地堡Component::OnBeforeDelayDelete()
{
	全都出地堡();
}

void 地堡Component::进(Space& refSpace, Entity& refEntity)
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

	if (refEntity.m_spPlayer)
		refEntity.m_spPlayer->m_refSession.删除选中(refEntity.Id);

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