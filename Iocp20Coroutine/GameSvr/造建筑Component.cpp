#include "pch.h"
#include "�콨��Component.h"
#include "��λ.h"
#include "AiCo.h"
#include "Entity.h"
#include "PlayerComponent.h"
#include "PlayerGateSession_Game.h"

void �콨��Component::AddComponet(Entity& refEntity, PlayerGateSession_Game& refGateSession, const ���λ���� ����)
{
}

�콨��Component::�콨��Component(PlayerGateSession_Game& refSession, Entity& refEntity, const ���λ���� ����):m_refEntity(refEntity)
{
	switch (����)
	{
	case ���̳�:
		m_set��������.insert(����); 
		m_set��������.insert(��);
		m_set��������.insert(����);
		m_set��������.insert(�ر�);
		break;
	default:
		break;
	}
}

CoTaskBool �콨��Component::Co�콨��(const Position &refPos, const ������λ���� ����)
{
	//���ߵ�Ŀ���
	if (co_await AiCo::WalkToPos(m_refEntity, refPos, m_cancel�콨��))
		co_return true;

	//Ȼ��ʼ��Ǯ����
	co_await m_refEntity.m_spPlayer->m_refSession.CoAddBuilding(����, refPos);
	co_return true;
}
