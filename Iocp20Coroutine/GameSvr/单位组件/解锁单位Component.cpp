#pragma once
#include "pch.h"
#include "������λComponent.h"
#include "����λComponent.h"
#include "BuildingComponent.h"
#include "../Entity.h"
#include "../../CoRoutine/CoTimer.h"
#include "../Space.h"
#include "../EntitySystem.h"
#include "../PlayerGateSession_Game.h"

������λComponent::������λComponent(Entity& ref) :m_refEntity(ref)
{
}

void ������λComponent::AddComponent(Entity& refEntity)
{
	refEntity.m_up������λ.reset(new ������λComponent(refEntity));
}

void ������λComponent::TryCancel()
{
	if (m_cancel������λ)
		m_cancel������λ();
}


void ������λComponent::������λ(const ��λ���� ����)
{
	CHECK_RET_VOID(m_refEntity.m_spBuilding);
	if (!m_refEntity.m_spBuilding->�����())
	{
		//��������
        PlayerGateSession_Game::��������Buzz(m_refEntity, "��û��ý���");
		return;
	}

	auto& spacePlayer = m_refEntity.m_refSpace.GetSpacePlayer(m_refEntity);
	if (!spacePlayer.��ʼ������λ(����, m_refEntity))
		return;

	Co������λ(����).RunNew();
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
	auto wp = GetPlayerGateSession(EntitySystem::GetNickName( m_refEntity));
	if (!wp.expired())
		wp.lock()->Send�ѽ�����λ();

	co_return true;
}