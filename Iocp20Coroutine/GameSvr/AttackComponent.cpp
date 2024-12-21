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
#include "��λ.h"

extern std::unordered_map<int, uint64_t> m_mapEntityId;
void AttackComponent::AddComponent(Entity& refEntity, const ���λ���� ����, const float f��������, const float f�˺�, const float f�������)
{
	CHECK_VOID(!refEntity.m_spAttack);
	refEntity.m_spAttack = std::make_shared<AttackComponent, Entity&, const ���λ����>(refEntity, std::forward<const ���λ����&&>(����));
	refEntity.m_spAttack->m_f�������� = f��������;
	refEntity.m_spAttack->m_f������� = f�������;
	refEntity.m_spAttack->m_f�˺� = f�˺�;
	//float arrF[] = { refEntity.Pos().x,0,refEntity.Pos().z};
	//int CrowToolAddAgent(float arrF[]);
	//refEntity.m_spAttack->m_idxCrowdAgent = CrowToolAddAgent(arrF);
	//m_mapEntityId[refEntity.m_spAttack->m_idxCrowdAgent] = refEntity.Id;
	refEntity.m_spAttack->Co����().RunNew();
}

float AttackComponent::��������(const Entity& refTarget) const
{
	const float fĿ�꽨����߳� = BuildingComponent::������߳�(refTarget);

	if (m_refEntity.m_wpOwner.expired())
		return m_f�������� + fĿ�꽨����߳�;//��ͨս����λ

	auto spOwner = m_refEntity.m_wpOwner.lock();
	return m_f�������� + fĿ�꽨����߳� + BuildingComponent::������߳�(*spOwner);
}

Position �����й�(const Position& refOld)
{
	auto posTarget = refOld;
	posTarget.x += std::rand() % 11 - 5;//�����
	posTarget.z += std::rand() % 11 - 5;
	return posTarget;
}

AttackComponent::AttackComponent(Entity& refEntity, const ���λ���� ����) :
	m_refEntity(refEntity),
	m_����(����),
	m_fun��������˴�(�����й�)
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

		Entity& refTarget = *wpEntity.lock();

		if (m_refEntity.DistanceLessEqual(refTarget, ��������(refTarget)))
		{
			��Component::Cancel���а�����·��Э��(m_refEntity); //TryCancel();

			if (co_await CoAttack(wpEntity, m_cancelAttack))
				co_return true;

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


CoTaskBool AttackComponent::CoAttack(WpEntity wpDefencer, FunCancel& cancel)
{
	KeepCancel kc(cancel);

	if (m_refEntity.IsDead())
		co_return false;//�Լ�����

	m_refEntity.BroadcastChangeSkeleAnim("attack");//���Ź�������

	using namespace std;

	const std::tuple<std::chrono::milliseconds, int> arrWaitHurt[] =
	{	//�����˺�{ÿ��ǰҡʱ�����˺�ֵ}
		//{300ms,2},
		{900ms,m_f�˺�},
		//{50ms,5}
	};

	switch (m_����)
	{
	case ��:
		if (!m_refEntity.m_spPlayer)
			EntitySystem::Broadcast��������(m_refEntity, "TMaFir00");
		break;
	case ��ս��:EntitySystem::Broadcast��������(m_refEntity, "Tfrshoot"); break;
	case ���̳�:EntitySystem::Broadcast��������(m_refEntity, "TSCMin00"); break;
	}

	for (auto wait_hurt : arrWaitHurt)
	{
		if (co_await CoTimer::Wait(std::get<0>(wait_hurt), cancel))//��x��	ǰҡ
			co_return true;//Э��ȡ��

		if (m_refEntity.IsDead())
			co_return false;//�Լ����������ٺ�ҡ

		if (wpDefencer.expired())
			break;//Ҫִ�к�ҡ

		auto spDefencer = wpDefencer.lock();
		if (spDefencer->IsDead())
			break;//Ҫִ�к�ҡ

		if (!m_refEntity.DistanceLessEqual(*spDefencer, ��������(*spDefencer)))
			break;//Ҫִ�к�ҡ

		if (!spDefencer->m_spDefence)
			break;//Ŀ�����

		if (m_refEntity.m_spPlayer && m_���� == ��)
			EntitySystem::Broadcast��������(m_refEntity, "��Ч/TTaFir00");

		spDefencer->m_spDefence->����(std::get<1>(wait_hurt));//��n���öԷ���
	}

	if (co_await CoTimer::Wait(800ms, cancel))//��ҡ
		co_return true;//Э��ȡ��

	if (!m_refEntity.IsDead())
	{
		EntitySystem::BroadcastChangeSkeleAnimIdle(m_refEntity);//�������д�������
	}

	co_return false;//Э�������˳�
}