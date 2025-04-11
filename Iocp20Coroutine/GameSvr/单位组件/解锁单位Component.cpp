#pragma once
#include "pch.h"
#include "������λComponent.h"
#include "����λComponent.h"
#include "../Entity.h"
#include "../../CoRoutine/CoTimer.h"
#include "../Space.h"
#include "../EntitySystem.h"

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
	co_return true;
}