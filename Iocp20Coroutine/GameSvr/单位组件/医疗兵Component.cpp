#include "pch.h"
#include "ҽ�Ʊ�Component.h"
#include "../Entity.h"
#include "../EntitySystem.h"
#include "../Space.h"
#include "../CoRoutine/CoTimer.h"
#include "../AiCo.h"
#include "��Component.h"
#include "�ɼ�Component.h"
#include "AttackComponent.h"
#include "BuildingComponent.h"
#include "��Ŀ���߹�ȥComponent.h"
#include "DefenceComponent.h"


ҽ�Ʊ�Component::ҽ�Ʊ�Component(Entity& refEntity) :m_refEntity(refEntity)
{
	m_refEntity.m_up��Ŀ���߹�ȥ->Co����(
		[this]()->bool {return this->��������(); },
		[this]()->WpEntity {return Get����Ŀ������ѷ���λ(); },
		[this](const Entity& refTarget, WpEntity wpEntity, ��Ŀ���߹�ȥComponent& ref��Ŀ���߹�ȥ)->CoTask<std::tuple<bool, bool>> {return Co����(refTarget, wpEntity, ref��Ŀ���߹�ȥ); },
		[this](WpEntity& wpEntity, bool ref���Ŀ��)->void {}
	).RunNew();
}

void ҽ�Ʊ�Component::AddComponent(Entity& refEntity)
{
	CHECK_VOID(!refEntity.m_upҽ�Ʊ�);
	refEntity.m_upҽ�Ʊ�.reset(new ҽ�Ʊ�Component(refEntity));
}


bool ҽ�Ʊ�Component::��������()
{
	//if (m_refEntity.m_spBuilding && !m_refEntity.m_spBuilding->�����())
	//	return false;

	if (m_refEntity.IsDead())
		return false;

	//if (�콨��Component::���ڽ���(m_refEntity))
	//	return false;

	if (m_refEntity.m_sp��)
	{
		if (!m_refEntity.m_sp��->m_coWalk�ֶ�����.Finished() ||
			!m_refEntity.m_sp��->m_coWalk���ر�.Finished())
			return false;//��ʾ��������
	}
	return true;
}


CoTask<std::tuple<bool, bool>> ҽ�Ʊ�Component::Co����(const Entity& refTarget, WpEntity wpEntity, ��Ŀ���߹�ȥComponent& ref��Ŀ���߹�ȥ)
{
	if (!m_refEntity.DistanceLessEqual(refTarget, ref��Ŀ���߹�ȥ.��������(refTarget)) || !ref��Ŀ���߹�ȥ.��鴩ǽ(refTarget))
		co_return{ false, false };

	��Component::Cancel���а�����·��Э��(m_refEntity); //TryCancel();

	if (co_await Co����Ŀ��(wpEntity, m_cancel����))
		co_return{ true, false };

	co_return{ false, true };
}

WpEntity ҽ�Ʊ�Component::Get����Ŀ������ѷ���λ()
{
	return m_refEntity.Get�����Entity֧�ֵر��еĵ�λ(Entity::�ѷ�, [this](const Entity& ref)->bool
		{
			if (!ref.m_spDefence)
				return false;
			
			if(ref.m_spDefence->����Ѫ())
                return false;

			return true;
		});
}


void ҽ�Ʊ�Component::TryCancel()
{
	if (m_cancel����)
	{
		//LOG(INFO) << "����m_cancel";
		m_cancel����();
	}
}

#define CHECK_��ֹ����Ŀ������ \
		if (m_refEntity.IsDead())\
			co_return false;\
		if (wpĿ��.expired())\
			break;\
		if (wpĿ��.lock()->IsDead())\
			break;\

CoTaskBool ҽ�Ʊ�Component::Co����Ŀ��(WpEntity wpĿ��, FunCancel& cancel)
{
	KeepCancel kc(cancel);
	//���λ��ʱ�赲 _(m_refEntity);
	EntitySystem::�ָ����ж��� _(m_refEntity, {});

	do
	{
		CHECK_��ֹ����Ŀ������;

		m_refEntity.m_eulerAnglesY = CalculateAngle(m_refEntity.Pos(), wpĿ��.lock()->Pos());
		m_refEntity.BroadcastNotifyPos();
		��Ŀ���߹�ȥComponent::����ǰҡ����(m_refEntity);

		CHECK_CO_RET_FALSE(m_refEntity.m_up��Ŀ���߹�ȥ);
		using namespace std;
		if (0s < m_refEntity.m_up��Ŀ���߹�ȥ->m_ս������.dura��ʼ���Ź������� && co_await CoTimer::Wait(m_refEntity.m_up��Ŀ���߹�ȥ->m_ս������.dura��ʼ���Ź�������, cancel))
			co_return true;//Э��ȡ��

		CHECK_��ֹ����Ŀ������;

		��Ŀ���߹�ȥComponent::���Ź�������(m_refEntity);
		const auto u16��ʼ�˺� = EntitySystem::�����󹥻�ǰҡ_�˺���ʱ(m_refEntity);
		if (0 < u16��ʼ�˺� && co_await CoTimer::Wait(std::chrono::milliseconds(u16��ʼ�˺�), cancel))
			co_return true;//Э��ȡ��

		CHECK_��ֹ����Ŀ������;

		auto& refDefencer = *wpĿ��.lock();
		CHECK_CO_RET_FALSE(m_refEntity.m_up��Ŀ���߹�ȥ);
		if (!m_refEntity.DistanceLessEqual(refDefencer, m_refEntity.m_up��Ŀ���߹�ȥ->��������(refDefencer)))
			break;//Ҫִ�к�ҡ

		if (!refDefencer.m_spDefence)
			break;//Ŀ�����

		��Ŀ���߹�ȥComponent::���Ź�����Ч(m_refEntity);

		const uint16_t u16������Ĺ��� = EntitySystem::�����󹥻�(m_refEntity);
		if (0 < u16������Ĺ���)
			refDefencer.m_spDefence->����(u16������Ĺ���, m_refEntity.Id);

	} while (false);

	CHECK_CO_RET_FALSE(m_refEntity.m_up��Ŀ���߹�ȥ);
	if (co_await CoTimer::Wait(m_refEntity.m_up��Ŀ���߹�ȥ->m_ս������.dura��ҡ, cancel))//��ҡ
		co_return true;//Э��ȡ��

	if (!m_refEntity.IsDead())
	{
		EntitySystem::BroadcastChangeSkeleAnimIdle(m_refEntity);//�������д�������
	}

	co_return false;//Э�������˳�
}