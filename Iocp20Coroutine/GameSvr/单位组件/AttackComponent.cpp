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
#include "����Ŀ��Component.h"

extern std::unordered_map<int, uint64_t> m_mapEntityId;
AttackComponent& AttackComponent::AddComponent(Entity& refEntity)
{
	if (refEntity.m_spAttack)
	{
		LOG(ERROR) << "�����ظ���AttackComponent";
		_ASSERT(!"�����ظ���AttackComponent");
		return *refEntity.m_spAttack;
	}
	refEntity.m_spAttack = std::make_shared<AttackComponent, Entity&>(refEntity);
	//refEntity.m_spAttack->m_f�������� = f��������;
	//refEntity.m_spAttack->m_f������� = f�������;
	//refEntity.m_spAttack->m_f�˺� = f�˺�;
	//float arrF[] = { refEntity.Pos().x,0,refEntity.Pos().z};
	//int CrowToolAddAgent(float arrF[]);
	//refEntity.m_spAttack->m_idxCrowdAgent = CrowToolAddAgent(arrF);
	//m_mapEntityId[refEntity.m_spAttack->m_idxCrowdAgent] = refEntity.Id;
	refEntity.m_spAttack->Co����().RunNew();
	return *refEntity.m_spAttack;
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
Position AttackComponent::�����й�(const Position& refOld)
{
	auto posTarget = refOld;
	posTarget.x += std::rand() % 11 - 5;//�����
	posTarget.z += std::rand() % 11 - 5;
	return posTarget;
}
using namespace std;
AttackComponent::AttackComponent(Entity& refEntity) :
	m_refEntity(refEntity)
{
	CHECK_RET_VOID(��λ::Findս������(refEntity.m_����, m_ս������));
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

		if (!���Թ���())
			continue;

		if ((m_b�����µ�Ŀ��) && co_await Co���򾯽䷶Χ�ڵ�Ŀ��Ȼ�󹥻�())
			continue;

		if (!m_bԭ�ؼ��� && m_refEntity.m_sp�� && m_fun��������˴� && !��Component::������(m_refEntity))//����������һ��Ŀ��
		{
			��Component::Cancel���а�����·��Э��(m_refEntity); //TryCancel();

			auto posTarget = m_fun��������˴�(m_refEntity.Pos());
			m_refEntity.m_refSpace.CrowdToolFindNerestPos(posTarget);
			if (m_refEntity.Pos().DistanceLessEqual(posTarget, 3))
			{
				m_fun��������˴� = m_refEntity.m_spPlayerNickName ? nullptr : �����й�;
				continue;
			}

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
	if (m_refEntity.m_spBuilding && !m_refEntity.m_spBuilding->�����())
		return false;

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

bool AttackComponent::��鴩ǽ(const Position& pos)
{
	if (��ɫ̹�� != m_refEntity.m_����)
		return true;

	return m_refEntity.m_refSpace.CrowdTool����ֱ��(m_refEntity.Pos(), pos);
}

CoTaskBool AttackComponent::Co���򾯽䷶Χ�ڵ�Ŀ��Ȼ�󹥻�()
{
	KeepCancel kc(m_TaskCancel.cancel);
	while (true)
	{
		if (co_await CoTimer::WaitNextUpdate(m_TaskCancel.cancel))
			co_return true;

		if (!���Թ���())
			co_return false;

		auto wpEntity = m_refEntity.Get�����Entity֧�ֵر��еĵ�λ(Entity::�з�, [this](const Entity& ref)->bool
			{
				if (!EntitySystem::Is�յ��ܴ�(m_refEntity.m_����, ref.m_����))
					return false;
				if (!ref.m_spDefence)
					return false;
				return nullptr != ref.m_spDefence;
			});
		if (wpEntity.expired())
		{
			m_b�����µ�Ŀ�� = false;//���䷶Χ��û��Ŀ��
			co_return false;
		}

		const bool b�����ѷ���λ̫�� = EntitySystem::�����ѷ���λ̫��(m_refEntity);
		Entity& refTarget = *wpEntity.lock();

		if (m_refEntity.DistanceLessEqual(refTarget, ��������(refTarget)) && !b�����ѷ���λ̫�� && ��鴩ǽ(refTarget.Pos()))
		{
			��Component::Cancel���а�����·��Э��(m_refEntity); //TryCancel();

			if (m_refEntity.m_���� == ��ɫ̹��)
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

		if (m_bԭ�ؼ���)
			continue;

		bool b���Ŀ�� = false;

		//����б�
		if (m_refEntity.m_spDefence)
		{
			auto& refMap = m_refEntity.m_spDefence->m_map�����˺�;
			while (!refMap.empty())
			{
				auto iterBegin = refMap.begin();
				WpEntity wp = m_refEntity.m_refSpace.GetEntity(iterBegin->first);
				if (wp.expired())
				{
					refMap.erase(iterBegin);
					continue;
				}

				auto& refEntity = *wp.lock();
				if (!m_refEntity.IsEnemy(refEntity))
				{
					refMap.erase(iterBegin);
					continue;
				}
				if (!m_refEntity.DistanceLessEqual(refEntity, 35))
				{
					refMap.erase(iterBegin);
					continue;
				}

				//wpEntity = wp;
				b���Ŀ�� = true;
				break;
			}
		}

		if (m_refEntity.m_wpOwner.expired() && (b���Ŀ�� || m_refEntity.DistanceLessEqual(refTarget, m_refEntity.�������())) &&
			//!��Component::������(m_refEntity) && 
			(!m_refEntity.m_sp�ɼ� || m_refEntity.m_sp�ɼ�->m_TaskCancel.co.Finished()))
		{
			��Component::Cancel���а�����·��Э��(m_refEntity); //TryCancel();

			if (m_refEntity.m_sp�ɼ�)
			{
				m_refEntity.m_sp�ɼ�->m_TaskCancel.TryCancel();
			}

			if (co_await AiCo::WalkToTarget(m_refEntity, wpEntity.lock(), m_TaskCancel.cancel, !b���Ŀ��))
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
	EntitySystem::�ָ����ж��� _(m_refEntity, {});

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

		switch (m_refEntity.m_����)
		{
		case ��̨:
			m_refEntity.Broadcast<Msg������Ч>({ .idEntity = m_refEntity.Id, .idEntityTarget = wpDefencer.lock()->Id,.��Чname = StrConv::GbkToUtf8("��Ч/������") });
			break;
		default:; break;
		}

		CHECK_��ֹ����Ŀ������;

		auto& refDefencer = *wpDefencer.lock();
		if (!m_refEntity.DistanceLessEqual(refDefencer, ��������(refDefencer)))
			break;//Ҫִ�к�ҡ

		if (!refDefencer.m_spDefence)
			break;//Ŀ�����

		���Ź�����Ч();

		if (0 < m_ս������.i32�˺�)
			refDefencer.m_spDefence->����(m_ս������.i32�˺�, m_refEntity.Id);

		if (��ɫ̹�� == m_refEntity.m_����)
		{
			auto wp��� = m_refEntity.m_refSpace.����λ(m_refEntity.m_spPlayer, EntitySystem::GetNickName(m_refEntity), m_refEntity.Pos(), ���);
			CHECK_WP_CO_RET_FALSE(wp���);
			����Ŀ��Component::AddComponet(*wp���.lock(), m_refEntity.Pos(),  (refDefencer.Pos()-m_refEntity.Pos()).��һ��(), m_ս������.f��������);
		}
	} while (false);

	if (co_await CoTimer::Wait(m_ս������.dura��ҡ, cancel))//��ҡ
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
		EntitySystem::BroadcastEntity����(m_refEntity, "׼������");
		{
			SpEntity spEntity��Ч = std::make_shared<Entity, const Position&, Space&, ��λ����, const ��λ::��λ����&>(
				posTarget, m_refEntity.m_refSpace, ��Ч, { "ը��" ,"��Ч/ը��","" });
			m_refEntity.m_refSpace.AddEntity(spEntity��Ч, 0);
			spEntity��Ч->BroadcastEnter();
			spEntity��Ч->CoDelayDelete(m_ս������.dura��ʼ���Ź������� + m_ս������.dura��ʼ�˺�).RunNew();
		}
		if (0s < m_ս������.dura��ʼ���Ź������� && co_await CoTimer::Wait(m_ս������.dura��ʼ���Ź�������, cancel))
			co_return true;//Э��ȡ��

		CHECK_��ֹ����λ������;
		EntitySystem::BroadcastEntity����(m_refEntity, "");
		���Ź�������();
		if (0s < m_ս������.dura��ʼ�˺� && co_await CoTimer::Wait(m_ս������.dura��ʼ�˺�, cancel))
			co_return true;//Э��ȡ��

		CHECK_��ֹ����λ������;

		if (!m_refEntity.Pos().DistanceLessEqual(posTarget, ��������(fĿ�꽨����߳�)))
			break;//Ҫִ�к�ҡ

		���Ź�����Ч();
		{
			SpEntity spEntity��Ч = std::make_shared<Entity, const Position&, Space&, ��λ����, const ��λ::��λ����&>(
				posTarget, m_refEntity.m_refSpace, ��Ч, { "��ը����" ,"��Ч/�ƹⱬ��","" });
			m_refEntity.m_refSpace.AddEntity(spEntity��Ч, 0);
			spEntity��Ч->BroadcastEnter();
			spEntity��Ч->CoDelayDelete(1s).RunNew();
		}
		_ASSERT(m_refEntity.m_upAoi);
		if (m_refEntity.m_upAoi)
		{
			auto& refUpAoi = m_refEntity.m_wpOwner.expired() ? m_refEntity.m_upAoi : m_refEntity.m_wpOwner.lock()->m_upAoi;
			CHECK_NOTNULL_CO_RET_FALSE(refUpAoi);
			for (auto [k, wp] : refUpAoi->m_map���ܿ�����)
			{
				CHECK_WP_CONTINUE(wp);
				auto& refDefencer = *wp.lock();
				if (!EntitySystem::Is�յ��ܴ�(m_refEntity.m_����, refDefencer.m_����))
					continue;

				if (refDefencer.m_spDefence && refDefencer.Pos().DistanceLessEqual(posTarget, 5))
					refDefencer.m_spDefence->����(m_ս������.i32�˺�, m_refEntity.Id);
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