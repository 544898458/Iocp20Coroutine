#include "pch.h"
#include "��Component.h"
#include "Entity.h"
#include "PlayerComponent.h"
#include "AttackComponent.h"
#include "AiCo.h"
#include "MonsterComponent.h"
#include "Space.h"
#include "�ر�Component.h"
#include "�ɼ�Component.h"
#include "�콨��Component.h"
#include "BuildingComponent.h"
#include "��Ŀ���߹�ȥComponent.h"
#include "ҽ�Ʊ�Component.h"

void ��Component::AddComponent(Entity& refEntity)
{
	CHECK_VOID(!refEntity.m_sp��);
	refEntity.m_sp�� = std::make_shared<��Component, Entity&>(refEntity);
	//float arrF[] = { refEntity.Pos().x,0,refEntity.Pos().z};
	//int CrowToolAddAgent(float arrF[]);
	//refEntity.m_spAttack->m_idxCrowdAgent = CrowToolAddAgent(arrF);
	//m_mapEntityId[refEntity.m_spAttack->m_idxCrowdAgent] = refEntity.Id;
}

��Component::��Component(Entity& refEntity) :m_refEntity(refEntity)
{
}

bool ��Component::������(const Entity& refEntity)
{
	if (!refEntity.m_sp��)
		return false;

	return !refEntity.m_sp��->m_coWalk.Finished();
}

void ��Component::WalkToTarget(SpEntity spTarget)
{
	_ASSERT(!m_cancel);
	_ASSERT(m_coWalk.Finished());
	m_coWalk = AiCo::WalkToTarget(m_refEntity, spTarget, m_cancel);
	m_coWalk.Run();
}

bool ��Component::WalkToTarget(Entity& refThis, SpEntity spTarget)
{
	CHECK_RET_FALSE(refThis.m_sp��);
	refThis.m_sp��->WalkToTarget(spTarget);
	return true;
}

void ��Component::WalkToPos(const Position& posTarget)
{
	_ASSERT(!m_cancel);
	_ASSERT(m_coWalk.Finished());
	m_coWalk = AiCo::WalkToPos(m_refEntity, posTarget, m_cancel);
	m_coWalk.Run();//Э���뿪��ʼ���У����е���һ��co_await
}

bool ��Component::WalkToPos(Entity& refThis, const Position& posTarget)
{
	CHECK_RET_FALSE(refThis.m_sp��);
	refThis.m_sp��->WalkToPos(posTarget);
	return true;
}

void ��Component::WalkToPos�ֶ�����(const Position& posTarget)
{
	//void CrowdToolSetMoveTarget(const float* p, const int idx);
	//float arrFloat[] = { posTarget.x,0,posTarget.z };
	//CrowdToolSetMoveTarget(arrFloat, refEntity.m_spAttack->m_idxCrowdAgent);
	//return;
	if (m_refEntity.IsDead())
	{
		//PlayerComponent::Say(m_refEntity, "�Լ�����,������", SayChannel::ϵͳ);

		return;
	}
	//m_coStop = true;

	TryCancel();

	m_coWalk.Run();
	_ASSERT(m_coWalk.Finished());//20240205
	if (m_refEntity.m_spAttack)
	{
		m_refEntity.m_spAttack->TryCancel();
		//_ASSERT(m_refEntity.m_spAttack->m_coAttack.Finished());//20240205
	}
	/*m_coStop = false;*/
	_ASSERT(!m_cancel);
	m_coWalk�ֶ����� = AiCo::WalkToPos(m_refEntity, posTarget, m_cancel);
	m_coWalk�ֶ�����.Run();//Э���뿪��ʼ���У����е���һ��co_await
}


void ��Component::TryCancel()
{
	if (m_cancel)
	{
		//LOG(INFO) << "����m_cancel";
		m_cancel();
	}
	else
	{
		//LOG(INFO) << "m_cancel�ǿյģ�û��Ҫȡ����Э��";
		if (!m_coWalk.Finished() || !m_coWalk�ֶ�����.Finished())
		{
			LOG(ERROR) << "Э��û������ȴ��ǰ�����m_cancel";
			_ASSERT(false);
		}
	}

	_ASSERT(m_coWalk.Finished());//20240205
	//_ASSERT(m_coAttack.Finished());//20240205

}

void ��Component::�߽��ر�(WpEntity wpEntity�ر�)
{
	if (m_refEntity.m_spAttack)
		m_refEntity.m_spAttack->TryCancel();

	if (m_refEntity.m_up��Ŀ���߹�ȥ)
		m_refEntity.m_up��Ŀ���߹�ȥ->TryCancel();

	if (m_refEntity.m_upҽ�Ʊ�)
		m_refEntity.m_upҽ�Ʊ�->TryCancel();

	if (wpEntity�ر�.expired())
		return;

	if (!wpEntity�ر�.lock()->m_spBuilding)
		return;

	if (!wpEntity�ر�.lock()->m_spBuilding->�����())
	{
		PlayerComponent::��������Buzz(m_refEntity, "�ر���û��ã����ܽ��ر�");
		return;
	}

	if (�콨��Component::���ڽ���(m_refEntity))
	{
		PlayerComponent::��������Buzz(m_refEntity, "���ڽ��죬���ܽ��ر�");
		return;
	}

	if (m_refEntity.m_spAttack)
		m_refEntity.m_spAttack->TryCancel();

	Cancel���а�����·��Э��(m_refEntity);
	_ASSERT(!m_cancel);
	_ASSERT(m_coWalk���ر�.Finished());
	m_coWalk���ر� = Co�߽��ر�(wpEntity�ر�);
	m_coWalk���ر�.Run();
}

CoTaskBool ��Component::Co�߽��ر�(WpEntity wpEntity�ر�)
{
	KeepCancel kc(m_cancel);

	if (m_refEntity.m_spAttack)
	{
		switch (m_refEntity.m_����)
		{
		case ���̳�:PlayerComponent::��������(m_refEntity, "����/����Ů���ɰ���", ""); break;
		case ǹ��:PlayerComponent::��������(m_refEntity, "����/��������������", ""); break;
		default:break;
		}
	}
	while (!wpEntity�ر�.expired())
	{
		if (m_refEntity.IsDead())
			co_return false;

		auto spEntity�ر� = wpEntity�ر�.lock();
		if (!spEntity�ر�->m_sp�ر�)
			co_return false;

		if (m_refEntity.DistanceLessEqual(*spEntity�ر�, m_refEntity.��������() + BuildingComponent::������߳�(*spEntity�ر�)))
		{
			spEntity�ر�->m_sp�ر�->��(m_refEntity.m_refSpace, m_refEntity);

			co_return false;
		}

		_ASSERT(!m_cancel);
		if (co_await AiCo::WalkToTarget(m_refEntity, spEntity�ر�, m_cancel, false))
			co_return true;

	}

	co_return false;
}

void ��Component::Cancel���а�����·��Э��(Entity& refEntity, const bool bֹͣ����)
{
	if (bֹͣ���� && refEntity.m_spAttack)refEntity.m_spAttack->TryCancel();
	if (refEntity.m_sp�ɼ�)		refEntity.m_sp�ɼ�->m_TaskCancel.TryCancel();
	if (refEntity.m_sp��)		refEntity.m_sp��->TryCancel();
	if (refEntity.m_sp�콨��)	refEntity.m_sp�콨��->TryCancel();
}