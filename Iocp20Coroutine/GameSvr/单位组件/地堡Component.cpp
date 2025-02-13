#include "pch.h"
#include "�ر�Component.h"
#include "AttackComponent.h"
#include "Entity.h"
#include "Space.h"
#include "EntitySystem.h"
#include "PlayerComponent.h"
#include "AoiComponent.h"
#include "PlayerGateSession_Game.h"

void �ر�Component::AddComponet(Entity& refEntity)
{
	refEntity.m_sp�ر� = std::make_shared<�ر�Component,Entity&>(refEntity);
}

void �ر�Component::OnDestroy()
{
	for (auto& sp : m_listSpEntity)
		sp->OnDestroy();

	m_listSpEntity.clear();
}

void �ر�Component::OnBeforeDelayDelete()
{
	ȫ�����ر�();
}

void �ر�Component::��(Space& refSpace, Entity& refEntity)
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

	if (refEntity.m_spPlayer)
		refEntity.m_spPlayer->m_refSession.ɾ��ѡ��(refEntity.Id);

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