#include "pch.h"
#include "AttackComponent.h"
#include "Entity.h"
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
void AttackComponent::AddComponent(Entity& refEntity, const ���λ���� ����)
{
	CHECK_VOID(!refEntity.m_spAttack);
	refEntity.m_spAttack = std::make_shared<AttackComponent, Entity&, const ���λ����>(refEntity, std::forward<const ���λ����&&>(����));
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

inline AttackComponent::AttackComponent(Entity& refEntity, const ���λ���� ����) :m_refEntity(refEntity), m_����(����)
{

}

void AttackComponent::TryCancel()
{
	if (m_cancel)
	{
		//LOG(INFO) << "����m_cancel";
		m_cancel();
	}
	else
	{
		//LOG(INFO) << "m_cancel�ǿյģ�û��Ҫȡ����Э��";
		if (!m_coAttack.Finished() || (m_refEntity.m_spMonster && !m_refEntity.m_spMonster->m_coIdle.Finished()))
		{
			LOG(ERROR) << "Э��û������ȴ��ǰ�����m_cancel";
			assert(false);
		}
	}

	//assert(m_coWalk.Finished());//20240205
	assert(m_coAttack.Finished());//20240205

}

void AttackComponent::Update()
{
	if (!m_coAttack.Finished())
		return;//��ʾ��������

	//if (!m_coWalk�ֶ�����.Finished())
	//	return;//��ʾ��������

	if (m_refEntity.IsDead())
		return;

	std::vector<std::pair<int64_t, SpEntity>> vecEnemy;
	std::copy_if(m_refEntity.m_refSpace.m_mapEntity.begin(), m_refEntity.m_refSpace.m_mapEntity.end(), std::back_inserter(vecEnemy), [this](const auto& pair)
		{
			auto& sp = pair.second;
			return sp.get() != &m_refEntity && !sp->IsDead() && sp->IsEnemy(m_refEntity);
		});

	auto iterMin = std::min_element(vecEnemy.begin(), vecEnemy.end(), [this](const auto& pair1, const auto& pair2)
		{
			auto& sp1 = pair1.second;
			auto& sp2 = pair2.second;
			return m_refEntity.DistancePow2(*sp1) < m_refEntity.DistancePow2(*sp2);
		});

	const auto& wpEntity = m_refEntity.m_refSpace.Get�����Entity(m_refEntity, true, [](const Entity& ref)->bool {return nullptr != ref.m_spDefence; });
	if (!wpEntity.expired())
	{
		Entity& refTarget = *wpEntity.lock();
		if (m_refEntity.DistanceLessEqual(refTarget, ��������()))
		{
			��Component::Cancel���а�����·��Э��(m_refEntity); //TryCancel();

			m_coAttack = CoAttack(wpEntity.lock(), m_cancel);
			m_coAttack.Run();
			return;
		}
		else if (m_refEntity.m_wpOwner.expired() && m_refEntity.DistanceLessEqual(refTarget, m_refEntity.m_f�������) && !��Component::������(m_refEntity) && (!m_refEntity.m_sp�ɼ� || m_refEntity.m_sp�ɼ�->m_TaskCancel.co.Finished()))
		{
			��Component::Cancel���а�����·��Э��(m_refEntity); //TryCancel();

			//m_coWalk.Run();
			//assert(m_coWalk.Finished());//20240205
			assert(m_coAttack.Finished());//20240205
			/*m_coStop = false;*/
			if (m_refEntity.m_sp�ɼ�)
			{
				m_refEntity.m_sp�ɼ�->m_TaskCancel.TryCancel();
			}

			��Component::WalkToTarget(m_refEntity, wpEntity.lock());

			return;
		}
	}

	if (!m_refEntity.m_spPlayer && !��Component::������(m_refEntity))//�������
	{
		��Component::Cancel���а�����·��Э��(m_refEntity); //TryCancel();
		//assert(m_coWalk.Finished());//20240205
		assert(m_coAttack.Finished());//20240205

		auto posTarget = m_refEntity.m_Pos;
		posTarget.x += std::rand() % 11 - 5;//�����
		posTarget.z += std::rand() % 11 - 5;
		//m_coWalk = AiCo::WalkToPos(m_refEntity.shared_from_this(), posTarget, m_cancel);
		��Component::WalkToPos(m_refEntity, posTarget);
	}
}


CoTask<int> AttackComponent::CoAttack(WpEntity wpDefencer, FunCancel& cancel)
{
	KeepCancel kc(cancel);

	if (m_refEntity.IsDead())
		co_return 0;//�Լ�����

	m_refEntity.BroadcastChangeSkeleAnim("attack");//���Ź�������

	using namespace std;

	const std::tuple<std::chrono::milliseconds, int> arrWaitHurt[] =
	{	//�����˺�{ÿ��ǰҡʱ�����˺�ֵ}
		{300ms,1},
		{50ms,3},
		{50ms,10}
	};

	for (auto wait_hurt : arrWaitHurt)
	{
		if (co_await CoTimer::Wait(std::get<0>(wait_hurt), cancel))//��x��	ǰҡ
			co_return 0;//Э��ȡ��

		if (m_refEntity.IsDead())
			co_return 0;//�Լ����������ٺ�ҡ

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
		co_return 0;//Э��ȡ��

	if (!m_refEntity.IsDead())
	{
		m_refEntity.BroadcastChangeSkeleAnim("idle");//�������д�������
	}

	co_return 0;//Э�������˳�
}