#pragma once
#include "pch.h"
#include "������λ����Component.h"
#include "����λComponent.h"
#include "BuildingComponent.h"
#include "������λComponent.h"
#include "../Entity.h"
#include "../../CoRoutine/CoTimer.h"
#include "../Space.h"
#include "../EntitySystem.h"
#include "../PlayerGateSession_Game.h"
#include "../ö��/��������.h"

������λ����Component::������λ����Component(Entity& ref) :m_refEntity(ref)
{
	switch (m_refEntity.m_����)
	{
	case ��Ӫ:
		m_map��������λ���� = { {ǹ��, {����}}, {��ս��, {����}} };
		break;
	case ��Ӫ:
		m_map��������λ���� = { {ǹ��, {����}}, {��ս��, {����}} };
		break;
	case �س���:
		m_map��������λ���� = { {��ɫ̹��, {�ƶ��ٶ�}} };
		break;
	case ����:
		m_map��������λ���� = { {�ɻ�, {����ǰҡ_�˺���ʱ}} };
		break;
	case ��̬Դ:
		m_map��������λ���� = { {��ɫ̹��, {����ǰҡ_�˺���ʱ}} };
		break;
	case ����:
		m_map��������λ���� = { {�ɳ�, {�ƶ��ٶ�}} };
		break;
	default:
		LOG(ERROR) << "����������λ����:" << m_refEntity.m_����;
		break;
	}
}

void ������λ����Component::AddComponent(Entity& refEntity)
{
	refEntity.m_up������λ����.reset(new ������λ����Component(refEntity));
}

void ������λ����Component::OnEntityDestroy(const bool bDestroy)
{
	m_cancel������λ����.TryCancel();
}


void ������λ����Component::����(const ��λ���� ��λ, const �������� ����)
{
	CHECK_RET_VOID(m_refEntity.m_upBuilding);
	if (!m_refEntity.m_upBuilding->�����())
	{
		PlayerGateSession_Game::��������Buzz(m_refEntity, "��û��ý���������������λ����");
		return;
	}
	if (��������())
	{
		PlayerGateSession_Game::��������Buzz(m_refEntity, "��������");
		return;
	}
	if (m_refEntity.m_up������λ && m_refEntity.m_up������λ->���ڽ���())
	{
		PlayerGateSession_Game::��������Buzz(m_refEntity, "���ڽ�����λ��������������");
		return;
	}

	{
		//�жϿ�������λ
		auto iterFind��λ = m_map��������λ����.find(��λ);
		if (iterFind��λ == m_map��������λ����.end())
		{
			PlayerGateSession_Game::��������Buzz(m_refEntity, "���������˵�λ������");
			return;
		}

		//�жϿ�����������
		auto iterFind���� = iterFind��λ->second.find(����);
		if (iterFind��λ->second.end() == iterFind����)
		{
			PlayerGateSession_Game::��������Buzz(m_refEntity, "���������˵�λ������");
			return;
		}
	}

	auto& spacePlayer = m_refEntity.m_refSpace.GetSpacePlayer(m_refEntity);

	if (!spacePlayer.��ʼ������λ����(��λ, ����, m_refEntity))
		return;

	Co����(��λ, ����).RunNew();
}

bool ������λ����Component::��������() const
{
	return m_cancel������λ����.operator bool();
}

CoTaskBool ������λ����Component::Co����(const ��λ���� ��λ, const �������� ����)
{
	const int MAX���� = 10;
	for (int i = 0; i < MAX����; ++i)
	{
		using namespace std;
		if (co_await CoTimer::Wait(1000ms, m_cancel������λ����))
			co_return false;

		EntitySystem::BroadcastEntity����(m_refEntity, std::format("��������{0}/{1}", i + 1, MAX����));
	}

	EntitySystem::BroadcastEntity����(m_refEntity, "");
	auto& spacePlayer = m_refEntity.m_refSpace.GetSpacePlayer(m_refEntity);
	spacePlayer.������λ�������(��λ, ����, m_refEntity);
	std::weak_ptr<PlayerGateSession_Game> GetPlayerGateSession(const std::string & refStrNickName);
	auto wp = GetPlayerGateSession(EntitySystem::GetNickName(m_refEntity));
	if (!wp.expired())
		wp.lock()->Send��λ����();

	co_return true;
}