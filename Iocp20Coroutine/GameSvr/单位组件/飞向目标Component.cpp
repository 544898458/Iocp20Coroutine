#include "pch.h"
#include "����Ŀ��Component.h"
#include "DefenceComponent.h"
#include "BuildingComponent.h"
#include "../Entity.h"
#include "../../CoRoutine/CoTimer.h"
#include "../Space.h"
#include "../EntitySystem.h"
void ����Ŀ��Component::AddComponet(Entity& refEntity, const Position& pos��ʼ��, const Position& pos����, const float f��Զ����)
{
	refEntity.m_up����Ŀ��.reset(new ����Ŀ��Component(refEntity, pos��ʼ��, pos����, f��Զ����));
}

����Ŀ��Component::����Ŀ��Component(Entity& ref, const Position& pos��ʼ��, const Position& vec����, const float f��Զ����) :
	m_refEntity(ref),
	m_pos��ʼ��(pos��ʼ��),
	m_vec����(vec����),
	m_f��Զ����(f��Զ����)
{
	Co����Ŀ�����б�ը().RunNew();
}

void ����Ŀ��Component::TryCancel()
{
	if (m_funCancel) {
		m_funCancel();
		m_funCancel = nullptr;
	}
}

CoTaskBool ����Ŀ��Component::Co����Ŀ�����б�ը()
{
	��λ::ս������ ����;
	CHECK_CO_RET_FALSE(��λ::Findս������(m_refEntity.m_����, ����));

	float f�״��ƶ����� = 3.f;
	while (!co_await CoTimer::WaitNextUpdate(m_funCancel))
	{
		auto wp = m_refEntity.Get�����Entity(Entity::�з�);
		if (!wp.expired())
		{
			auto &refĿ�� = *wp.lock();
			const auto f������߳� = BuildingComponent::������߳�(refĿ��);
			if (refĿ��.m_spDefence && refĿ��.Pos().DistanceLessEqual(m_refEntity.Pos(), 1 + f������߳�))
			{
				EntitySystem::Broadcast��������(m_refEntity, ����.str������Ч);
				refĿ��.m_spDefence->����(����.i32����, m_refEntity.Id);
				m_refEntity.DelayDelete();
				co_return false;
			}
		}

		if (!m_refEntity.Pos().DistanceLessEqual(m_pos��ʼ��, m_f��Զ����))//�������
		{
			m_refEntity.DelayDelete();
			co_return false;
		}

		const auto posNew = m_refEntity.Pos() + m_vec���� * ����.fÿ֡�ƶ����� * f�״��ƶ�����;
		f�״��ƶ����� = 1.f;
		if (!m_refEntity.m_refSpace.CrowdTool����ֱ��(posNew, m_refEntity.Pos()))//ײǽ
		{
			m_refEntity.DelayDelete();
			co_return false;
		}

		m_refEntity.SetPos(posNew);
		m_refEntity.BroadcastNotifyPos();
	}
	m_refEntity.DelayDelete();
	co_return true;
}