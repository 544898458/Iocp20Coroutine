#include "pch.h"
#include "�콨��Component.h"
#include "��λ.h"

void �콨��Component::AddComponet(Entity& refEntity, PlayerGateSession_Game& refGateSession, const ���λ���� ����)
{
}

�콨��Component::�콨��Component(PlayerGateSession_Game& refSession, Entity& refEntity, const ���λ���� ����)
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
