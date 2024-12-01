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
#include "../CoRoutine/CoTimer.h"
#include "DefenceComponent.h"

extern std::unordered_map<int, uint64_t> m_mapEntityId;
void AttackComponent::AddComponent(Entity& refEntity, const ���λ���� ����, const float f��������)
{
	CHECK_VOID(!refEntity.m_spAttack);
	refEntity.m_spAttack = std::make_shared<AttackComponent, Entity&, const ���λ����>(refEntity, std::forward<const ���λ����&&>(����));
	refEntity.m_spAttack->m_f�������� = f��������;
	//float arrF[] = { refEntity.m_Pos.x,0,refEntity.m_Pos.z};
	//int CrowToolAddAgent(float arrF[]);
	//refEntity.m_spAttack->m_idxCrowdAgent = CrowToolAddAgent(arrF);
	//m_mapEntityId[refEntity.m_spAttack->m_idxCrowdAgent] = refEntity.Id;
}

float AttackComponent::��������() const
{
	if (m_refEntity.m_wpOwner.expired())
		return m_f��������;

	auto spOwner = m_refEntity.m_wpOwner.lock();
	if (!spOwner->m_spAttack)
		return m_f��������;

	return spOwner->m_spAttack->m_f�������� + m_f��������;
}

Position �����й�(const Position &refOld)
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

	if(bDestroy)
		m_TaskCancel.TryCancel();
}

void AttackComponent::Update()
{
	if (m_refEntity.IsDead())
		return;

	m_TaskCancel.TryRun(Co(m_TaskCancel.cancel));
	if (!m_TaskCancel.co.Finished())
		return;//���ڹ��������򹥻�λ��

	if (!m_refEntity.m_spPlayer && !��Component::������(m_refEntity))//�������
	{
		��Component::Cancel���а�����·��Э��(m_refEntity); //TryCancel();
		//assert(m_coWalk.Finished());//20240205
		//assert(m_coAttack.Finished());//20240205


		auto posTarget = m_fun��������˴�(m_refEntity.m_Pos);
		//m_coWalk = AiCo::WalkToPos(m_refEntity.shared_from_this(), posTarget, m_cancel);
		��Component::WalkToPos(m_refEntity, posTarget);
		//co_await AiCo::WalkToPos(m_refEntity, posTarget, funCancel);
		return ;
	}
}

CoTaskBool AttackComponent::Co(FunCancel &funCancel)
{
	KeepCancel kc(funCancel);
	while (true) 
	{
		//if (co_await CoTimer::WaitNextUpdate(funCancel))
		//	co_return true;

		//if (!m_coAttack.Finished())
		//	continue;//��ʾ��������

		if (m_refEntity.m_sp�� && !m_refEntity.m_sp��->m_coWalk�ֶ�����.Finished())
			co_return false;//��ʾ��������

		if (m_refEntity.IsDead())
			co_return false;

		const auto& wpEntity = m_refEntity.m_refSpace.Get�����Entity(m_refEntity, true, [](const Entity& ref)->bool {return nullptr != ref.m_spDefence; });
		if (!wpEntity.expired())
		{
			Entity& refTarget = *wpEntity.lock();
			if (m_refEntity.DistanceLessEqual(refTarget, ��������()))
			{
				��Component::Cancel���а�����·��Э��(m_refEntity); //TryCancel();

				if (co_await CoAttack(wpEntity.lock(), m_cancelAttack))
					co_return true;

				continue;
			}
			else if (m_refEntity.m_wpOwner.expired() && m_refEntity.DistanceLessEqual(refTarget, m_refEntity.m_f�������) &&
				//!��Component::������(m_refEntity) && 
				(!m_refEntity.m_sp�ɼ� || m_refEntity.m_sp�ɼ�->m_TaskCancel.co.Finished()))
			{
				��Component::Cancel���а�����·��Э��(m_refEntity); //TryCancel();

				if (m_refEntity.m_sp�ɼ�)
				{
					m_refEntity.m_sp�ɼ�->m_TaskCancel.TryCancel();
				}

				//��Component::WalkToTarget(m_refEntity, wpEntity.lock());
				if (co_await AiCo::WalkToTarget(m_refEntity, wpEntity.lock(), funCancel))
					co_return true;

				continue;
			}
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
		{300ms,1},
		{50ms,3},
		{50ms,5}
	};

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

		if (!m_refEntity.DistanceLessEqual(*spDefencer, ��������()))
			break;//Ҫִ�к�ҡ

		if (!spDefencer->m_spDefence)
			break;//Ŀ�����

		spDefencer->m_spDefence->����(std::get<1>(wait_hurt));//��n���öԷ���1
	}

	if (co_await CoTimer::Wait(1000ms, cancel))//��3��	��ҡ
		co_return true;//Э��ȡ��

	if (!m_refEntity.IsDead())
	{
		EntitySystem::BroadcastChangeSkeleAnimIdle(m_refEntity);//�������д�������
	}

	co_return false;//Э�������˳�
}