#include "pch.h"
#include "AttackComponent.h"
#include "Entity.h"
#include "EntitySystem.h"
#include "PlayerComponent.h"
#include "MonsterComponent.h"
#include "../IocpNetwork/StrConv.h"
#include "GameSvrSession.h"
#include "AiCo.h"
#include "Space.h"
#include "PlayerGateSession_Game.h"
#include "�ɼ�Component.h"
#include "��Component.h"
#include "�콨��Component.h"
#include "../CoRoutine/CoTimer.h"
#include "DefenceComponent.h"
#include "BuildingComponent.h"
#include "AoiComponent.h"
#include "��ʱ�赲Component.h"

extern std::unordered_map<int, uint64_t> m_mapEntityId;
void AttackComponent::AddComponent(Entity& refEntity, const ���λ���� ����, const ��λ::ս������& ����)
{
	CHECK_VOID(!refEntity.m_spAttack);
	refEntity.m_spAttack = std::make_shared<AttackComponent, Entity&, const ���λ����, const ��λ::ս������&>(
		refEntity, std::forward<const ���λ����&&>(����), ����);
	//refEntity.m_spAttack->m_f�������� = f��������;
	//refEntity.m_spAttack->m_f������� = f�������;
	//refEntity.m_spAttack->m_f�˺� = f�˺�;
	//float arrF[] = { refEntity.Pos().x,0,refEntity.Pos().z};
	//int CrowToolAddAgent(float arrF[]);
	//refEntity.m_spAttack->m_idxCrowdAgent = CrowToolAddAgent(arrF);
	//m_mapEntityId[refEntity.m_spAttack->m_idxCrowdAgent] = refEntity.Id;
	refEntity.m_spAttack->Co����().RunNew();
}

float AttackComponent::��������(const Entity& refTarget) const
{
	const float fĿ�꽨����߳� = BuildingComponent::������߳�(refTarget);
	return ��������(fĿ�꽨����߳�);
}
float AttackComponent::��������(const float fĿ�꽨����߳�) const
{
	if (m_refEntity.m_wpOwner.expired())
		return m_ս������.f�������� + fĿ�꽨����߳�;//��ͨս����λ

	auto spOwner = m_refEntity.m_wpOwner.lock();
	return m_ս������.f�������� + fĿ�꽨����߳� + BuildingComponent::������߳�(*spOwner);
}
Position �����й�(const Position& refOld)
{
	auto posTarget = refOld;
	posTarget.x += std::rand() % 11 - 5;//�����
	posTarget.z += std::rand() % 11 - 5;
	return posTarget;
}
using namespace std;
AttackComponent::AttackComponent(Entity& refEntity, const ���λ���� ����, const ��λ::ս������& ����) :
	m_refEntity(refEntity),
	m_����(����),
	m_fun��������˴�(�����й�),
	m_ս������(����)
{
}

void AttackComponent::TryCancel(const bool bDestroy)
{
	if (m_cancelAttack)
	{
		//LOG(INFO) << "����m_cancel";
		m_cancelAttack();
	}

	m_TaskCancel.TryCancel();

	if (bDestroy && m_funCancel����)
		m_funCancel����();
}

CoTaskBool AttackComponent::Co����()
{
	using namespace std;
	//while (!co_await CoTimer::Wait(1000ms, m_funCancel����))
	while (!co_await CoTimer::WaitNextUpdate(m_funCancel����))
	{
		if (m_refEntity.IsDead())
			co_return false;


		if (m_b�����µ�Ŀ�� && co_await Co���򾯽䷶Χ�ڵ�Ŀ��Ȼ�󹥻�(m_TaskCancel.cancel))
			continue;

		if (!m_refEntity.m_spPlayer && !��Component::������(m_refEntity))//�������
		{
			��Component::Cancel���а�����·��Э��(m_refEntity); //TryCancel();

			auto posTarget = m_fun��������˴�(m_refEntity.Pos());
			m_refEntity.m_refSpace.CrowdToolFindNerestPos(posTarget);
			��Component::WalkToPos(m_refEntity, posTarget);
			continue;
		}
	}
	co_return true;
}

void AttackComponent::Update()
{
}

bool AttackComponent::���Թ���()
{
	if (m_refEntity.m_sp�� && !m_refEntity.m_sp��->m_coWalk�ֶ�����.Finished())
		return false;//��ʾ��������

	if (m_refEntity.IsDead())
		return false;

	if (�콨��Component::���ڽ���(m_refEntity))
		return false;

	if (m_refEntity.m_sp��)
	{
		if (!m_refEntity.m_sp��->m_coWalk�ֶ�����.Finished() ||
			!m_refEntity.m_sp��->m_coWalk���ر�.Finished())
			return false;//��ʾ��������
	}
	return true;
}

CoTaskBool AttackComponent::Co���򾯽䷶Χ�ڵ�Ŀ��Ȼ�󹥻�(FunCancel& funCancel)
{
	KeepCancel kc(funCancel);
	while (true)
	{
		if (!���Թ���())
			co_return false;

		const auto wpEntity = m_refEntity.m_refSpace.Get�����Entity֧�ֵر��еĵ�λ(m_refEntity, true, [](const Entity& ref)->bool {return nullptr != ref.m_spDefence; });
		if (wpEntity.expired())
		{
			m_b�����µ�Ŀ�� = false;//���䷶Χ��û��Ŀ��
			co_return false;
		}

		const auto wp������ѷ���λ = m_refEntity.m_refSpace.Get�����Entity(m_refEntity, false, [](const Entity& ref)->bool {return nullptr != ref.m_spDefence; });
		bool b�����ѷ���λ̫�� = false;
		if (!wp������ѷ���λ.expired())
		{
			b�����ѷ���λ̫�� = m_refEntity.DistanceLessEqual(*wp������ѷ���λ.lock(), 0.6f);
		}
		Entity& refTarget = *wpEntity.lock();

		if (m_refEntity.DistanceLessEqual(refTarget, ��������(refTarget)) && !b�����ѷ���λ̫��)
		{
			��Component::Cancel���а�����·��Э��(m_refEntity); //TryCancel();

			if (m_���� == ��ɫ̹��)
			{
				if (co_await CoAttackλ��(refTarget.Pos(), BuildingComponent::������߳�(refTarget), m_cancelAttack))
					co_return true;
			}
			else
			{
				if (co_await CoAttackĿ��(wpEntity, m_cancelAttack))
					co_return true;
			}

			continue;
		}
		else if (m_refEntity.m_wpOwner.expired() && m_refEntity.DistanceLessEqual(refTarget, m_refEntity.�������()) &&
			//!��Component::������(m_refEntity) && 
			(!m_refEntity.m_sp�ɼ� || m_refEntity.m_sp�ɼ�->m_TaskCancel.co.Finished()))
		{
			��Component::Cancel���а�����·��Э��(m_refEntity); //TryCancel();

			if (m_refEntity.m_sp�ɼ�)
			{
				m_refEntity.m_sp�ɼ�->m_TaskCancel.TryCancel();
			}

			if (co_await AiCo::WalkToTarget(m_refEntity, wpEntity.lock(), funCancel))
				co_return true;

			continue;
		}

		co_return false;
	}

	co_return false;
}

void AttackComponent::����ǰҡ����()
{
	//switch (m_����)
	//{
	//case ��ɫ̹��:m_refEntity.BroadcastChangeSkeleAnim("attack_loop"); break;
	//default:m_refEntity.BroadcastChangeSkeleAnim("attack"); break;
	//}
	if (!m_ս������.strǰҡ����.empty())
		m_refEntity.BroadcastChangeSkeleAnim(m_ս������.strǰҡ����);
}

void AttackComponent::���Ź�������()
{
	//switch (m_����)
	//{
	//case ��ɫ̹��:m_refEntity.BroadcastChangeSkeleAnim("attack_loop"); break;
	//default:m_refEntity.BroadcastChangeSkeleAnim("attack"); break;
	//}
	if (!m_ս������.str��������.empty())
		m_refEntity.BroadcastChangeSkeleAnim(m_ս������.str��������);
}

void AttackComponent::���Ź�����Ч()
{
	//switch (m_����)
	//{
	//case ��:
	//	if (!m_refEntity.m_spPlayer)
	//		EntitySystem::Broadcast��������(m_refEntity, "TMaFir00");
	//	break;
	//case ��ս��:EntitySystem::Broadcast��������(m_refEntity, "Tfrshoot"); break;
	//case ���̳�:EntitySystem::Broadcast��������(m_refEntity, "TSCMin00"); break;
	//case ��ɫ̹��:EntitySystem::Broadcast��������(m_refEntity, "��Ч/TTaFi200"); break;
	//}

	if (!m_ս������.str������Ч.empty())
		EntitySystem::Broadcast��������(m_refEntity, m_ս������.str������Ч);
}

#define CHECK_��ֹ����Ŀ������ \
		if (m_refEntity.IsDead())\
			co_return false;\
		if (wpDefencer.expired())\
			break;\
		if (wpDefencer.lock()->IsDead())\
			break;\

CoTaskBool AttackComponent::CoAttackĿ��(WpEntity wpDefencer, FunCancel& cancel)
{
	KeepCancel kc(cancel);
	//���λ��ʱ�赲 _(m_refEntity);
	do
	{
		CHECK_��ֹ����Ŀ������;

		m_refEntity.m_eulerAnglesY = CalculateAngle(m_refEntity.Pos(), wpDefencer.lock()->Pos());
		m_refEntity.BroadcastNotifyPos();
		����ǰҡ����();

		if (0s < m_ս������.dura��ʼ���Ź������� && co_await CoTimer::Wait(m_ս������.dura��ʼ���Ź�������, cancel))
			co_return true;//Э��ȡ��

		CHECK_��ֹ����Ŀ������;

		���Ź�������();
		if (0s < m_ս������.dura��ʼ�˺� && co_await CoTimer::Wait(m_ս������.dura��ʼ�˺�, cancel))
			co_return true;//Э��ȡ��

		CHECK_��ֹ����Ŀ������;

		auto& refDefencer = *wpDefencer.lock();
		if (!m_refEntity.DistanceLessEqual(refDefencer, ��������(refDefencer)))
			break;//Ҫִ�к�ҡ

		if (!refDefencer.m_spDefence)
			break;//Ŀ�����

		���Ź�����Ч();

		refDefencer.m_spDefence->����(m_ս������.f�˺�);
	} while (false);

	if (co_await CoTimer::Wait(800ms, cancel))//��ҡ
		co_return true;//Э��ȡ��

	if (!m_refEntity.IsDead())
	{
		EntitySystem::BroadcastChangeSkeleAnimIdle(m_refEntity);//�������д�������
	}

	co_return false;//Э�������˳�
}


#define CHECK_��ֹ����λ������ \
		if (m_refEntity.IsDead())\
			co_return false;\

CoTaskBool AttackComponent::CoAttackλ��(const Position posTarget, const float fĿ�꽨����߳�, FunCancel& cancel)
{
	KeepCancel kc(cancel);
	//���λ��ʱ�赲 _(m_refEntity);

	do
	{
		CHECK_��ֹ����λ������;

		m_refEntity.m_eulerAnglesY = CalculateAngle(m_refEntity.Pos(), posTarget);
		m_refEntity.BroadcastNotifyPos();
		����ǰҡ����();

		if (0s < m_ս������.dura��ʼ���Ź������� && co_await CoTimer::Wait(m_ս������.dura��ʼ���Ź�������, cancel))
			co_return true;//Э��ȡ��

		CHECK_��ֹ����λ������;

		���Ź�������();
		if (0s < m_ս������.dura��ʼ�˺� && co_await CoTimer::Wait(m_ս������.dura��ʼ�˺�, cancel))
			co_return true;//Э��ȡ��

		CHECK_��ֹ����λ������;

		if (!m_refEntity.Pos().DistanceLessEqual(posTarget, ��������(fĿ�꽨����߳�)))
			break;//Ҫִ�к�ҡ

		���Ź�����Ч();
		{
			SpEntity spEntity��Ч = std::make_shared<Entity, const Position&, Space&, const ��λ::��λ����&>(
				posTarget, m_refEntity.m_refSpace, { "��Ч" ,"��Ч/�ƹⱬ��","" });
			m_refEntity.m_refSpace.AddEntity(spEntity��Ч, 0);
			spEntity��Ч->BroadcastEnter();
			spEntity��Ч->CoDelayDelete().RunNew();
		}
		assert(m_refEntity.m_upAoi);
		if (m_refEntity.m_upAoi)
		{
			for (auto [k, wp] : m_refEntity.m_upAoi->m_map���ܿ�����)
			{
				CHECK_WP_CONTINUE(wp);
				auto& refDefencer = *wp.lock();
				if (refDefencer.m_spDefence && refDefencer.Pos().DistanceLessEqual(posTarget, 5))
					refDefencer.m_spDefence->����(m_ս������.f�˺�);
			}
		}

	} while (false);

	if (co_await CoTimer::Wait(800ms, cancel))//��ҡ
		co_return true;//Э��ȡ��

	if (!m_refEntity.IsDead())
	{
		EntitySystem::BroadcastChangeSkeleAnimIdle(m_refEntity);//�������д�������
	}

	co_return false;//Э�������˳�
}