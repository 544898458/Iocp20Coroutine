#include "pch.h"
#include "�ر�Component.h"
#include "AttackComponent.h"
#include "Entity.h"
#include "Space.h"
#include "EntitySystem.h"
#include "PlayerComponent.h"
#include "AoiComponent.h"
#include "PlayerGateSession_Game.h"

void �ر�Component::AddComponent(Entity& refEntity)
{
	CHECK_RET_VOID(!refEntity.m_up�ر�);
	refEntity.m_up�ر�.reset(new �ر�Component(refEntity));
}


void �ɽ����λComponent::AddComponent(Entity& refEntity)
{
    refEntity.AddComponentOnDestroy(&Entity::m_up�ɽ����λ, refEntity);
}

void �ɽ����λComponent::OnEntityDestroy(const bool bDestroy)
{
	for (auto& sp : m_listSpEntity)
		sp->OnDestroy();

	m_listSpEntity.clear();
}

void �ɽ����λComponent::OnBeforeDelayDelete()
{
	ȫ����ȥ();
}

void �ɽ����λComponent::��(Space& refSpace, Entity& refEntity)
{
	if (m_refEntity.IsDead())
	{
		PlayerComponent::Say(m_refEntity, "�ر��Ѵݻٲ����ٽ���", SayChannel::ϵͳ);
		return;
	}
	if (m_listSpEntity.size() >= 10)
	{
		PlayerComponent::Say(m_refEntity, "�ر�������", SayChannel::ϵͳ);
		return;
	}

	//�ӵ�ͼ��ɾ������¼�ڵر���
	if (refEntity.IsDead())
	{
		LOG(INFO) << "������λ���ܽ��ر�";//_ASSERTfalse);
		return;
	}
	refEntity.SetPos(m_refEntity.Pos());
	refEntity.BroadcastLeave();//OnDestroy();
	if (refEntity.m_upAoi)
		refEntity.m_upAoi->�뿪Space();

	m_listSpEntity.push_back(refEntity.shared_from_this());
	refSpace.m_mapEntity.erase(refEntity.Id);
	
	refEntity.m_wpOwner = m_refEntity.weak_from_this();

	if (refEntity.m_upPlayer)
		refEntity.m_upPlayer->m_refSession.ɾ��ѡ��(refEntity.Id);

	EntitySystem::BroadcastEntity����(m_refEntity, std::format("�ر�����{0}��", m_listSpEntity.size()));
}

void �ɽ����λComponent::ȫ����ȥ()
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

	EntitySystem::BroadcastEntity����(m_refEntity, std::format("�ر��ǿյ�", m_listSpEntity.size()));
}