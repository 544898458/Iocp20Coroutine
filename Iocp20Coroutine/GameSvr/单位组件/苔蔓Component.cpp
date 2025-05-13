#include "pch.h"
#include "̦��Component.h"
#include "AoiComponent.h"
#include "BuffComponent.h"
#include "Entity.h"
#include "DefenceComponent.h"
#include "��ֵComponent.h"
#include "../../CoRoutine/CoTimer.h"
#include "../EntitySystem.h"
#include "../ö��/BuffId.h"
#include "../ö��/��������.h"

void ̦��Component::AddComponent(Entity& refEntity)
{
	refEntity.AddComponentOnDestroy(&Entity::m_up̦��, refEntity);
}

̦��Component::̦��Component(Entity& ref) :m_refEntity(ref)
{
	Coή������().RunNew();
	Co����Χ��Buff().RunNew();
}

void ̦��Component::OnEntityDestroy(const bool bDestroy)
{
	m_funCancelή������.TryCancel();
	m_funCancel����Χ��Buff.TryCancel();
}

bool ̦��Component::�ڰ뾶��(const Position& pos) const
{
	return m_refEntity.Pos().DistanceLessEqual(pos, m_i16�뾶);
}

CoTaskBool ̦��Component::Coή������()
{
	using namespace std;
	while (!co_await CoTimer::Wait(10s, m_funCancelή������))
	{
		if (!m_wp���Ž���.expired())
			continue;

		m_i16�뾶 -= 1;
		EntitySystem::BroadcastEntity̦���뾶(m_refEntity);

		if (m_i16�뾶 < 0)
		{
			m_refEntity.DelayDelete();
			co_return false;
		}
	}
	co_return false;
}


CoTaskBool ̦��Component::Co����Χ��Buff()
{
	using namespace std;
	while (!co_await CoTimer::Wait(1000ms, m_funCancel����Χ��Buff))
	{
		if (m_refEntity.IsDead())
			co_return false;

		CHECK_CO_RET_FALSE(m_refEntity.m_upAoi);
		for (auto& [_, wp] : m_refEntity.m_upAoi->m_map���ܿ�����)
		{
			CHECK_WP_CO_RET_FALSE(wp);
			auto& refEntity = *wp.lock();
			if (!EntitySystem::Is���λ������(refEntity))
				continue;

			if (!m_refEntity.DistanceLessEqual(refEntity, m_i16�뾶))
				continue;


			if (!refEntity.m_upBuff)
				continue;

			if (!refEntity.m_upDefence)
				continue;

			if (��λ::Is��(refEntity.m_����) && !m_refEntity.IsEnemy(refEntity))//ֻ���ѷ����嵥λ���ٶ�
			{
				refEntity.m_upBuff->������(̦�����浥λ���ƶ��ٶ�);
				refEntity.m_upBuff->ɾBuff(�뿪̦���ĳ潨��������Ѫ);
				��ֵComponent::�ı�(refEntity, ����, 1);
			}
			else if (EntitySystem::Is����(refEntity))//�����ཨ����Ѫ�����ֵ��ң�
			{
				refEntity.m_upDefence->����(1, m_refEntity.Id);
			}
		}
	}
	co_return false;
}

bool ̦��Component::���Ž���()const
{
	return m_i16�뾶 >= MAX�뾶;
}