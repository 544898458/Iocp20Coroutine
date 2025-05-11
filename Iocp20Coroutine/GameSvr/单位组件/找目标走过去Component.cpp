#include "pch.h"
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
#include "AoiComponent.h"

��Ŀ���߹�ȥComponent::��Ŀ���߹�ȥComponent(Entity& refEntity) :m_refEntity(refEntity)
{
	CHECK_RET_VOID(��λ::Findս������(refEntity.m_����, m_ս������));
}

void ��Ŀ���߹�ȥComponent::AddComponent(Entity& refEntity)
{
	refEntity.AddComponentOnDestroy(&Entity::m_up��Ŀ���߹�ȥ, refEntity);
}

float ��Ŀ���߹�ȥComponent::��������(const Entity& refTarget) const
{
	const float fĿ�꽨����߳� = BuildingComponent::������߳�(refTarget);
	return ��������(fĿ�꽨����߳�);
}
float ��Ŀ���߹�ȥComponent::��������(const float fĿ�꽨����߳�) const
{
	CHECK_RET_DEFAULT(m_refEntity.m_up��Ŀ���߹�ȥ);
	if (m_refEntity.m_wpOwner.expired())
		return BuildingComponent::������߳�(m_refEntity) + m_refEntity.m_up��Ŀ���߹�ȥ->m_ս������.f�������� + fĿ�꽨����߳�;//��ͨս����λ

	auto spOwner = m_refEntity.m_wpOwner.lock();
	return BuildingComponent::������߳�(*spOwner) + m_refEntity.m_up��Ŀ���߹�ȥ->m_ս������.f�������� + fĿ�꽨����߳�;
}

void ��Ŀ���߹�ȥComponent::�����ѭ��(
	const std::function<bool()> fun���Բ���,
	const std::function<WpEntity()> fun�������Ŀ��,
	const std::function<CoTask<std::tuple<bool, bool>>(const Entity& refTarget, WpEntity wpEntity, ��Ŀ���߹�ȥComponent& ref��Ŀ���߹�ȥ)> fun���������Ŀ��,
	const std::function<void(WpEntity& wpEntity, bool& ref���Ŀ��)> fun������Ŀ��)
{
	m_TaskCancel����.TryRun(CoTaskBool(Co����(fun���Բ���, fun�������Ŀ��, fun���������Ŀ��, fun������Ŀ��)));
}

CoTaskBool ��Ŀ���߹�ȥComponent::Co����(
	const std::function<bool()> fun���Բ���,
	const std::function<WpEntity()> fun�������Ŀ��,
	const std::function<CoTask<std::tuple<bool, bool>>(const Entity& refTarget, WpEntity wpEntity, ��Ŀ���߹�ȥComponent& ref��Ŀ���߹�ȥ)> fun���������Ŀ��,
	const std::function<void(WpEntity& wpEntity, bool& ref���Ŀ��)> fun������Ŀ��)
{
	using namespace std;
	//while (!co_await CoTimer::Wait(1000ms, m_funCancel����))
	KeepCancel kc(m_TaskCancel����.cancel);
	bool b�ⲿȡ��Э�� = false;
	FunCancel funCancel = [&b�ⲿȡ��Э��]()
		{
			b�ⲿȡ��Э�� = true;
		};
	m_TaskCancel����.cancel = funCancel;
	while (!co_await CoTimer::WaitNextUpdate(m_TaskCancel����.cancel))
	{
		if (b�ⲿȡ��Э��)
			co_return true;

		if (m_refEntity.IsDead())
			co_return false;

		if (!m_refEntity.m_wpOwner.expired() && ���� == m_refEntity.m_wpOwner.lock()->m_����)
			continue;

		if (!fun���Բ���())
			continue;

		if (m_b�����µ�Ŀ�� && co_await Co���򾯽䷶Χ�ڵ�Ŀ��Ȼ�����(fun���Բ���, fun�������Ŀ��, fun���������Ŀ��, fun������Ŀ��))
		{
			if (b�ⲿȡ��Э��)
				co_return true;

			continue;//������������⣬�޷��������ֶ�ֹͣ��·��ϻ������ٶ�����
		}
		if (m_refEntity.m_wpOwner.expired() && !m_bԭ�ؼ��� && m_refEntity.m_up�� && m_fun��������˴� && !��Component::������(m_refEntity) && !�ɼ�Component::���ڲɼ�(m_refEntity))//����������һ��Ŀ��
		{
			��Component::Cancel���а�����·��Э��(m_refEntity); //TryCancel();

			auto posTarget = m_fun��������˴�(m_refEntity.Pos());
			m_refEntity.m_refSpace.CrowdToolFindNerestPos(posTarget);
			if (m_refEntity.Pos().DistanceLessEqual(posTarget, 3))
			{
				m_fun��������˴� = m_refEntity.m_upPlayerNickName ? nullptr : �����й�;
				continue;
			}

			��Component::WalkToPos(m_refEntity, posTarget);
			continue;
		}
	}
	co_return true;
}


CoTaskBool ��Ŀ���߹�ȥComponent::Co���򾯽䷶Χ�ڵ�Ŀ��Ȼ�����(
	const std::function<bool()> fun���Բ���,
	const std::function<WpEntity()> fun�������Ŀ��,
	const std::function<CoTask<std::tuple<bool, bool>>(const Entity& refTarget, WpEntity wpEntity, ��Ŀ���߹�ȥComponent& ref��Ŀ���߹�ȥ)> fun���������Ŀ��,
	const std::function<void(WpEntity& wpEntity, bool& ref���Ŀ��)> fun������Ŀ��)
{
	CHECK_CO_RET_FALSE(m_refEntity.m_upAoi);
	KeepCancel kc(m_TaskCancel.cancel);
	bool b�ⲿȡ��Э��(false);
	FunCancel funCancel = [&b�ⲿȡ��Э��]()
		{
			b�ⲿȡ��Э�� = true;
		};
	m_TaskCancel.cancel = funCancel;
	while (true)
	{
		if (co_await CoTimer::WaitNextUpdate(m_TaskCancel.cancel))
			co_return true;

		if (!fun���Բ���())
			co_return false;

		//CHECK_CO_RET_FALSE(m_refEntity.m_upAoi);
		//if (m_refEntity.m_upAoi->m_map���ܿ�����.empty())
		//{
		//	m_b�����µ�Ŀ�� = false;//AOI�ھ��䷶Χ��û��Ŀ��
		//	co_return false;
		//}

		auto wpEntity = fun�������Ŀ��();
		if (wpEntity.expired())
		{
			co_return false;
		}

		{
			Entity& refTarget = *wpEntity.lock();
			const auto [bStop, bContinue] = co_await fun���������Ŀ��(refTarget, wpEntity, *this);
			if (b�ⲿȡ��Э��)
				co_return true;

			if (bStop)
				co_return true;

			if (bContinue)
				continue;

			if (m_bԭ�ؼ���)
				continue;
		}

		bool b���Ŀ�� = false;

		fun������Ŀ��(wpEntity, b���Ŀ��);

		if (wpEntity.expired())
		{
			//m_b�����µ�Ŀ�� = false;//���䷶Χ��û��Ŀ��
			co_return false;
		}

		Entity& refTarget = *wpEntity.lock();

		if (m_refEntity.m_wpOwner.expired() && (b���Ŀ�� || m_refEntity.DistanceLessEqual(refTarget, m_refEntity.�������())) &&
			//!��Component::������(m_refEntity) && 
			(!m_refEntity.m_up�ɼ� || m_refEntity.m_up�ɼ�->m_TaskCancel.co.Finished())
			)
		{
			��Component::Cancel���а�����·��Э��(m_refEntity); //TryCancel();

			if (m_refEntity.m_up�ɼ�)
			{
				m_refEntity.m_up�ɼ�->m_TaskCancel.TryCancel();
			}

			if (m_refEntity.m_up��)//��̨û�������
			{
				if (co_await m_refEntity.m_up��->WalkToTarget(wpEntity.lock(), m_TaskCancel.cancel, !b���Ŀ��, [this](Entity& ref) {return ��鴩ǽ(ref); }))
					co_return true;
			}
			continue;//�������ߵ����������ڣ��ٹ�������
		}

		co_return false;
	}

	co_return false;
}

bool ��Ŀ���߹�ȥComponent::��鴩ǽ(const Entity& refEntity)
{
	if (��ɫ̹�� != m_refEntity.m_����)
		return true;

	auto pos����Ŀ��� = refEntity.Pos();
	const auto f������߳� = BuildingComponent::������߳�(refEntity);
	//��λ::ս������ ս������;
	if (0 < f������߳�)//&& ��λ::Findս������(m_refEntity.m_����, ս������))
	{
		const auto vecֱ�� = refEntity.Pos() - m_refEntity.Pos();

		if (vecֱ��.Length() <= f������߳�)
			return true;

		const auto vec���� = vecֱ��.��һ��();
		const auto f�ߵ�������Χ�� = vecֱ��.Length() - f������߳�;
		pos����Ŀ��� = m_refEntity.Pos() + vec���� * f�ߵ�������Χ��;
	}

	if (!m_refEntity.m_refSpace.CrowdTool����ֱ��(m_refEntity.Pos(), pos����Ŀ���))
		return false;

	return true;
}

void ��Ŀ���߹�ȥComponent::OnEntityDestroy(const bool bDestroy)
{
	if (bDestroy)
	{
		m_TaskCancel����.TryCancel();
	}

	m_TaskCancel.TryCancel();
}


Position ��Ŀ���߹�ȥComponent::�����й�(const Position& refOld)
{
	auto posTarget = refOld;
	posTarget.x += std::rand() % 11 - 5;//�����
	posTarget.z += std::rand() % 11 - 5;
	return posTarget;
}

void ��Ŀ���߹�ȥComponent::����ǰҡ����(Entity& refEntity)
{
	CHECK_RET_VOID(refEntity.m_up��Ŀ���߹�ȥ);
	if (!refEntity.m_up��Ŀ���߹�ȥ->m_ս������.strǰҡ����.empty())
		refEntity.BroadcastChangeSkeleAnim(refEntity.m_up��Ŀ���߹�ȥ->m_ս������.strǰҡ����);
}


void ��Ŀ���߹�ȥComponent::���Ź�������(Entity& refEntity)
{
	CHECK_RET_VOID(refEntity.m_up��Ŀ���߹�ȥ);
	if (!refEntity.m_up��Ŀ���߹�ȥ->m_ս������.str��������.empty())
		refEntity.BroadcastChangeSkeleAnim(refEntity.m_up��Ŀ���߹�ȥ->m_ս������.str��������);
}


void ��Ŀ���߹�ȥComponent::���Ź�����Ч(Entity& refEntity)
{
	//switch (m_����)
	//{
	//case ��:
	//	if (!m_refEntity.m_upPlayer)
	//		EntitySystem::Broadcast��������(m_refEntity, "TMaFir00");
	//	break;
	//case ��ս��:EntitySystem::Broadcast��������(m_refEntity, "Tfrshoot"); break;
	//case ���̳�:EntitySystem::Broadcast��������(m_refEntity, "TSCMin00"); break;
	//case ��ɫ̹��:EntitySystem::Broadcast��������(m_refEntity, "��Ч/TTaFi200"); break;
	//}

	CHECK_RET_VOID(refEntity.m_up��Ŀ���߹�ȥ);
	if (!refEntity.m_up��Ŀ���߹�ȥ->m_ս������.str������Ч.empty())
		EntitySystem::Broadcast��������(refEntity, refEntity.m_up��Ŀ���߹�ȥ->m_ս������.str������Ч);
}