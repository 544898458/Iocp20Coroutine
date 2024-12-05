#include "pch.h"
#include "�ر�Component.h"
#include "AttackComponent.h"
#include "Entity.h"
#include "Space.h"
#include "EntitySystem.h"
#include "PlayerComponent.h"

void �ر�Component::AddComponet(Entity& refEntity, PlayerGateSession_Game& refGateSession)
{
	refEntity.m_sp�ر� = std::make_shared<�ر�Component,Entity&>(refEntity);
}

void �ر�Component::OnDestroy()
{
	m_listSpEntity.clear();
}

void �ر�Component::OnBeforeDelayDelete()
{
	ȫ�����ر�();
}

void �ر�Component::��(Space& refSpace, uint64_t idEntity)
{
	if (m_listSpEntity.size() >= 10)
	{
		PlayerComponent::Say(m_refEntity, "�ر�������", SayChannel::ϵͳ);
		return;
	}

	//�ӵ�ͼ��ɾ������¼�ڵر���
	auto wp = refSpace.GetEntity(idEntity);
	CHECK_RET_VOID(!wp.expired());
	auto sp = wp.lock();
	if (sp->IsDead())
	{
		LOG(INFO) << "������λ���ܽ��ر�";//assert(false);
		return;
	}
	sp->BroadcastLeave();//OnDestroy();
	sp->m_wpOwner = sp->weak_from_this();
	m_listSpEntity.push_back(sp);
	refSpace.m_mapEntity.erase(idEntity);
	//sp->BroadcastLeave();
	EntitySystem::BroadcastEntity����(m_refEntity, std::format("�ر�����{0}��", m_listSpEntity.size()));
}

void �ر�Component::Update()
{
	for (auto& sp : m_listSpEntity) 
	{
		if (sp->m_spAttack)
			sp->m_spAttack->Update();
	}
}

void �ر�Component::ȫ�����ر�()
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