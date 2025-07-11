#include "pch.h"
#include "����Ŀ��Component.h"
#include "DefenceComponent.h"
#include "BuildingComponent.h"
#include "AttackComponent.h"
#include "../Entity.h"
#include "../../CoRoutine/CoTimer.h"
#include "../Space.h"
#include "../EntitySystem.h"
void ����Ŀ��Component::AddComponent(Entity& refEntity, const Position& pos��ʼ��, const Position& pos����, const float f��Զ����, const uint64_t idAttacker)
{
	refEntity.AddComponentOnDestroy(&Entity::m_up����Ŀ��, refEntity, pos��ʼ��, pos����, f��Զ����, idAttacker);
}

����Ŀ��Component::����Ŀ��Component(Entity& ref, const Position& pos��ʼ��, const Position& vec����, const float f��Զ����, const uint64_t idAttacker) :
	m_refEntity(ref),
	m_pos��ʼ��(pos��ʼ��),
	m_vec����(vec����),
	m_f��Զ����(f��Զ����),
	m_idAttacker(idAttacker),
	m_funCancel("����Ŀ��Component")
{
	Co����Ŀ�����б�ը().RunNew();
}

void ����Ŀ��Component::OnEntityDestroy(const bool bDestroy)
{
	m_funCancel.TryCancel();
}

CoTaskBool ����Ŀ��Component::Co����Ŀ�����б�ը()
{
	��λ::ս������ ����;
	CHECK_CO_RET_FALSE(��λ::Findս������(m_refEntity.m_����, ����));

	using namespace std;

	float f�״��ƶ����� = 3.f;
	while (!co_await CoTimer::WaitNextUpdate(m_funCancel))
	{
		auto wp = m_refEntity.Get�����Entity(Entity::�з�, [this](const Entity& ref)->bool {return EntitySystem::Is�յ��ܴ�(m_refEntity.m_����, ref.m_����);});
		if (!wp.expired())
		{
			auto &refĿ�� = *wp.lock();
			const auto f������߳� = BuildingComponent::������߳�(refĿ��);
			if (refĿ��.m_upDefence && refĿ��.Pos().DistanceLessEqual(m_refEntity.Pos(), 1 + f������߳�))
			{
				EntitySystem::Broadcast��������(m_refEntity, ����.str������Ч);
				refĿ��.m_upDefence->����(EntitySystem::�����󹥻�(m_refEntity), m_idAttacker);
				m_refEntity.DelayDelete(0s);
				co_return false;
			}
		}

		if (!m_refEntity.Pos().DistanceLessEqual(m_pos��ʼ��, m_f��Զ����))//�������
		{
			m_refEntity.DelayDelete(0s);
			co_return false;
		}

		const auto posNew = m_refEntity.Pos() + m_vec���� * ����.fÿ֡�ƶ����� * f�״��ƶ�����;
		f�״��ƶ����� = 1.f;
		//if (!m_refEntity.m_refSpace.CrowdTool����ֱ��(posNew, m_refEntity.Pos()))//ײǽ
		//{
		//	m_refEntity.DelayDelete();
		//	co_return false;
		//}

		m_refEntity.SetPos(posNew);
		m_refEntity.BroadcastNotifyPos();
	}
	m_refEntity.DelayDelete(0s);
	co_return true;
}