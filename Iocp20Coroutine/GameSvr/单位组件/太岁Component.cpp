#include "pch.h"
#include "̫��Component.h"
#include "̦��Component.h"
#include "̦������Component.h"
#include "�콨��Component.h"
#include "Entity.h"
#include "../../CoRoutine/CoTimer.h"
#include "../EntitySystem.h"
#include "BuffComponent.h"
#include "../ö��/BuffId.h"
#include "../PlayerGateSession_Game.h"

void ̫��Component::AddComponent(Entity& refEntity)
{
	LOG_IF(ERROR, refEntity.m_up̫��) << "m_up̫��";
	refEntity.m_up̫��.reset(new ̫��Component(refEntity));
}

void ̫��Component::����(const Position& refPos)
{
	if (!m_wp����.expired())
	{
		PlayerGateSession_Game::��������Buzz(m_refEntity, "�����ٷ���");
		return;
	}

	CHECK_RET_VOID(m_refEntity.m_up̦������);
	CHECK_WP_RET_VOID(m_refEntity.m_up̦������->m_wp̦��);
	const auto& ref̦�� = *m_refEntity.m_up̦������->m_wp̦��.lock();
	CHECK_RET_VOID(ref̦��.m_up̦��);
	;
	if (!ref̦��.m_up̦��->���Ž���())
	{
		PlayerGateSession_Game::��������Buzz(m_refEntity, "���ȵȴ�̫���̦��(w��n)���ŵ����Χ");
		return;
	}

	//�����ڱ�̦����Χ��
	if (!ref̦��.m_up̦��->�ڰ뾶��(refPos))
	{
		PlayerGateSession_Game::��������Buzz(m_refEntity, "�������ѡ��̫���̦��(w��n)��");
		return;
	}

	m_wp���� = �콨��Component::��������(m_refEntity.m_refSpace, refPos, ̫��, m_refEntity.m_upPlayer, EntitySystem::GetNickName(m_refEntity));
	CHECK_WP_RET_VOID(m_wp����);
}

̫��Component::̫��Component(Entity& ref) :m_refEntity(ref)
{

}
