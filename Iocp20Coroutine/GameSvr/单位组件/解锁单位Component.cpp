#pragma once
#include "pch.h"
#include "������λComponent.h"
#include "����λComponent.h"
#include "BuildingComponent.h"
#include "������λ����Component.h"
#include "../Entity.h"
#include "../../CoRoutine/CoTimer.h"
#include "../Space.h"
#include "../EntitySystem.h"
#include "../PlayerGateSession_Game.h"

������λComponent::������λComponent(Entity& ref) :m_refEntity(ref), m_cancel������λ("m_cancel������λ")
{
}

void ������λComponent::AddComponent(Entity& refEntity)
{
	refEntity.AddComponentOnDestroy(&Entity::m_up������λ, refEntity);
}

void ������λComponent::OnEntityDestroy(const bool bDestroy)
{
	m_cancel������λ.TryCancel();
}


void ������λComponent::������λ(const ��λ���� ����)
{
	CHECK_RET_VOID(m_refEntity.m_upBuilding);
	if (!m_refEntity.m_upBuilding->�����())
	{
		PlayerGateSession_Game::��������Buzz(m_refEntity, "��û��ý��������ܽ�����λ");
		return;
	}

	if (���ڽ���())
	{
		PlayerGateSession_Game::��������Buzz(m_refEntity, "���ڽ���");
		return;
	}

	if (m_refEntity.m_up������λ���� && m_refEntity.m_up������λ����->��������())
	{
		PlayerGateSession_Game::��������Buzz(m_refEntity, "�������������ܽ�����λ");
		return;
	}


	auto& spacePlayer = m_refEntity.m_refSpace.GetSpacePlayer(m_refEntity);
	if (!spacePlayer.��ʼ������λ(����, m_refEntity))
		return;

	Co������λ(����).RunNew();
}

bool ������λComponent::���ڽ���() const
{
	return m_cancel������λ.operator bool();
}

CoTaskBool ������λComponent::Co������λ(const ��λ���� ����)
{
	const int MAX���� = 10;
	for (int i = 0; i < MAX����; ++i)
	{
		using namespace std;
		if (co_await CoTimer::Wait(1000ms, m_cancel������λ))
			co_return false;

		EntitySystem::BroadcastEntity����(m_refEntity, std::format("��������{0}/{1}", i + 1, MAX����));
	}

	EntitySystem::BroadcastEntity����(m_refEntity, "");
	auto& spacePlayer = m_refEntity.m_refSpace.GetSpacePlayer(m_refEntity);
	spacePlayer.�������(����, m_refEntity);
	std::weak_ptr<PlayerGateSession_Game> GetPlayerGateSession(const std::string & refStrNickName);
	auto wp = GetPlayerGateSession(EntitySystem::GetNickName(m_refEntity));
	if (!wp.expired())
		wp.lock()->Send�ѽ�����λ();

	co_return true;
}