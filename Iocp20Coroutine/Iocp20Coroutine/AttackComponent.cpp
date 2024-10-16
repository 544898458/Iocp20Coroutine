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

extern std::unordered_map<int, uint64_t> m_mapEntityId;
void AttackComponent::AddComponent(Entity& refEntity)
{
	CHECK_VOID(!refEntity.m_spAttack);
	refEntity.m_spAttack = std::make_shared<AttackComponent>();
	//float arrF[] = { refEntity.m_Pos.x,0,refEntity.m_Pos.z};
	//int CrowToolAddAgent(float arrF[]);
	//refEntity.m_spAttack->m_idxCrowdAgent = CrowToolAddAgent(arrF);
	//m_mapEntityId[refEntity.m_spAttack->m_idxCrowdAgent] = refEntity.Id;
}

void AttackComponent::WalkToPos(Entity& refEntity, const Position& posTarget)
{
	//void CrowdToolSetMoveTarget(const float* p, const int idx);
	//float arrFloat[] = { posTarget.x,0,posTarget.z };
	//CrowdToolSetMoveTarget(arrFloat, refEntity.m_spAttack->m_idxCrowdAgent);
	//return;
	if (refEntity.IsDead())
	{
		if (refEntity.m_spPlayer)
			refEntity.m_spPlayer->Say("�Լ�����,������");

		return;
	}
	//m_coStop = true;

	TryCancel(refEntity);

	m_coWalk.Run();
	assert(m_coWalk.Finished());//20240205
	assert(m_coAttack.Finished());//20240205
	/*m_coStop = false;*/
	m_coWalk = AiCo::WalkToPos(refEntity.shared_from_this(), posTarget, m_cancel);
	m_coWalk.Run();//Э���뿪��ʼ���У����е���һ��co_await
}


void AttackComponent::TryCancel(Entity& refEntity)
{
	if (m_cancel)
	{
		//LOG(INFO) << "����m_cancel";
		m_cancel();
	}
	else
	{
		//LOG(INFO) << "m_cancel�ǿյģ�û��Ҫȡ����Э��";
		if (!m_coWalk.Finished() || !m_coAttack.Finished() || (refEntity.m_spMonster && !refEntity.m_spMonster->m_coIdle.Finished()))
		{
			LOG(ERROR) << "Э��û������ȴ��ǰ�����m_cancel";
			assert(false);
		}
	}

	assert(m_coWalk.Finished());//20240205
	assert(m_coAttack.Finished());//20240205

}

void AttackComponent::Update(Entity& refThis)
{
	if (!m_coAttack.Finished())
	{
		return;//��ʾ��������
	}
	if (!m_coWalk.Finished())
	{
		return;//��ʾ��������
	}

	if (refThis.IsDead())
	{
		return;
	}

	std::vector<std::pair<int64_t, SpEntity>> vecEnemy;
	std::copy_if(refThis.m_refSpace.m_mapEntity.begin(), refThis.m_refSpace.m_mapEntity.end(), std::back_inserter(vecEnemy), [&refThis](const auto& pair)
		{
			auto& sp = pair.second;
			return sp.get() != &refThis && !sp->IsDead() && sp->IsEnemy(refThis);
		});

	auto iterMin = std::min_element(vecEnemy.begin(), vecEnemy.end(), [&refThis](const auto& pair1, const auto& pair2)
		{
			auto& sp1 = pair1.second;
			auto& sp2 = pair2.second;
			return refThis.DistancePow2(*sp1) < refThis.DistancePow2(*sp2);
		});

	const auto& wpEntity = refThis.m_refSpace.Get�����Entity(refThis, true, [](const Entity& ref)->bool {return nullptr != ref.m_spDefence; });
	if (!wpEntity.expired())
	{
		if (refThis.DistanceLessEqual(*wpEntity.lock(), refThis.m_f��������))
		{
			TryCancel(refThis);

			m_coAttack = AiCo::Attack(refThis.shared_from_this(), wpEntity.lock(), m_cancel);
			m_coAttack.Run();
			return;
		}
		else if (refThis.DistanceLessEqual(*wpEntity.lock(), refThis.m_f�������))
		{
			TryCancel(refThis);

			//m_coWalk.Run();
			assert(m_coWalk.Finished());//20240205
			assert(m_coAttack.Finished());//20240205
			/*m_coStop = false;*/
			m_coWalk = AiCo::WalkToTarget(refThis.shared_from_this(), wpEntity.lock(), m_cancel);
			m_coWalk.Run();//Э���뿪��ʼ���У����е���һ��co_await
			return;
		}
	}

	if (!refThis.m_spPlayer)//�������
	{
		TryCancel(refThis);
		assert(m_coWalk.Finished());//20240205
		assert(m_coAttack.Finished());//20240205

		auto posTarget = refThis.m_Pos;
		posTarget.x += std::rand() % 11 - 5;//�����
		posTarget.z += std::rand() % 11 - 5;
		m_coWalk = AiCo::WalkToPos(refThis.shared_from_this(), posTarget, m_cancel);
		m_coWalk.Run();//Э���뿪��ʼ���У����е���һ��co_await
	}
}

