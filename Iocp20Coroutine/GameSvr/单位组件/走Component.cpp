#include "pch.h"
#include "��Component.h"
#include "../Entity.h"
#include "../EntitySystem.h"
#include "../../CoRoutine/CoTimer.h"
#include "../../CoRoutine/CoEvent.h"
#include "PlayerComponent.h"
#include "AttackComponent.h"
#include "AiCo.h"
#include "MonsterComponent.h"
#include "Space.h"
#include "�ɽ����λComponent.h"
#include "�ɼ�Component.h"
#include "�콨��Component.h"
#include "BuildingComponent.h"
#include "��Ŀ���߹�ȥComponent.h"
#include "ҽ�Ʊ�Component.h"
#include "��ʱ�赲Component.h"
#include "../RecastNavigationCrowd.h"

void ��Component::AddComponent(Entity& refEntity)
{
	CHECK_RET_VOID(!refEntity.m_up��);
	refEntity.AddComponentOnDestroy(&Entity::m_up��, refEntity);
	//float arrF[] = { refEntity.Pos().x,0,refEntity.Pos().z};
	//int CrowToolAddAgent(float arrF[]);
	//refEntity.m_upAttack->m_idxCrowdAgent = CrowToolAddAgent(arrF);
	//m_mapEntityId[refEntity.m_upAttack->m_idxCrowdAgent] = refEntity.Id;
}

��Component::��Component(Entity& refEntity) :m_refEntity(refEntity)
{
}

bool ��Component::������(const Entity& refEntity)
{
	if (!refEntity.m_up��)
		return false;

	return !refEntity.m_up��->m_coWalk.Finished();
}

void ��Component::WalkToTarget(SpEntity spTarget)
{
	_ASSERT(!m_cancel);
	_ASSERT(m_coWalk.Finished());
	m_coWalk = WalkToTarget(spTarget, m_cancel);
	m_coWalk.Run();
}

/// <summary>
/// 
/// </summary>
/// <param name="refThis"></param>
/// <param name="localTarget"></param>
/// <returns>�Ƿ�Ҫ����һ��</returns>
bool ���ߵ�Ŀ�긽��(Entity& refThis, const Position localTarget, const float f����Ŀ��С�ڴ˾���ͣ�� = 0)
{
	CHECK_RET_DEFAULT(refThis.m_up��);
	const float step = std::max(EntitySystem::�������ٶ�ÿ֡�ƶ�����(refThis), f����Ŀ��С�ڴ˾���ͣ��);
	if (refThis.Pos().DistanceLessEqual(localTarget, step))
	{
		//LOG(INFO) << "���ߵ�" << localTarget.x << "," << localTarget.z << "������Э�������˳�";
		//EntitySystem::BroadcastChangeSkeleAnimIdle(refThis);
		return true;
	}

	//refThis.m_eulerAnglesY = CalculateAngle(refThis.Pos(), localTarget);
	refThis.BroadcastNotifyPos();

	return false;
}

CoTaskBool ��Component::WalkToTarget(SpEntity spTarget, FunCancel& funCancel, const bool b��龯�����, const std::function<bool(Entity&)>& fun��ͣ��)
{
	//if (!refThis.m_up��)
	//	co_return false;

	if (m_refEntity.IsDead())
	{
		//LOG(WARNING) << m_refEntity.ͷ��Name() << ",������������";
		co_return false;
	}

	auto posTarget = spTarget->Pos();
	{
		const auto ok = m_refEntity.m_refSpace.CrowdToolFindNerestPos(posTarget);
		LOG_IF(ERROR, !ok) << "";
		_ASSERT(ok);
	}
	���λ����·���赲 _(m_refEntity);
	RecastNavigationCrowd rnc(m_refEntity, posTarget);
	KeepCancel kc(funCancel);
	const float f������߳� = BuildingComponent::������߳�(*spTarget);


	//refThis.BroadcastChangeSkeleAnim("run");
	��λ::���λ���� ����;
	��λ::Find���λ����(m_refEntity.m_����, ����);
	EntitySystem::�ָ����ж��� __(m_refEntity, ����.str��·����);
	Position posOld;
	CoEvent<MyEvent::MoveEntity>::OnRecvEvent({ m_refEntity.weak_from_this() });
	while (true)
	{
		if (co_await CoTimer::WaitNextUpdate(funCancel))//�������������̴߳�ѭ����ÿ��ѭ������һ��
		{
			//LOG(INFO) << "����" << spTarget << "��Э��ȡ����";
			co_return true;
		}
		if (m_refEntity.IsDead())
		{
			//LOG(INFO) << "�Լ�����������[" << spTarget->ͷ��Name() << "]��Э��ȡ����";
			//PlayerComponent::Say(m_refEntity, "�Լ�����", SayChannel::ϵͳ);

			co_return false;
		}
		if (b��龯����� && !m_refEntity.DistanceLessEqual(*spTarget, m_refEntity.�������()))
		{
			//LOG(INFO) << "�뿪�Լ��ľ������" << spTarget << "��Э��ȡ����";
			co_return false;
		}
		const bool b�����ѷ���λ̫�� = EntitySystem::�����ѷ���λ̫��(m_refEntity);
		if (!b�����ѷ���λ̫�� && m_refEntity.DistanceLessEqual(*spTarget, m_refEntity.��������() + f������߳�) && fun��ͣ��(*spTarget))
		{
			//LOG(INFO) << "���ߵ�" << spTarget << "������Э�������˳�";
			//EntitySystem::BroadcastChangeSkeleAnimIdle(m_refEntity);
			//EntitySystem::BroadcastEntity����(m_refEntity, "���ߵ�Ŀ�긽��");
			co_return false;
		}

		if (posOld != spTarget->Pos())
		{
			rnc.SetMoveTarget(spTarget->Pos());
			posOld = spTarget->Pos();
		}

		if (���ߵ�Ŀ�긽��(m_refEntity, spTarget->Pos()))
		{
			co_return false;
		}
		//EntitySystem::BroadcastEntity����(m_refEntity, std::format("��Ŀ��{0}��", (int)m_refEntity.Distance(*spTarget)));
	}
	LOG(INFO) << "����Ŀ��Э�̽���:" << m_refEntity.Pos();
	co_return false;
}

bool ��Component::WalkToTarget(Entity& refThis, SpEntity spTarget)
{
	CHECK_RET_FALSE(refThis.m_up��);
	refThis.m_up��->WalkToTarget(spTarget);
	return true;
}

void ��Component::WalkToPos(const Position& posTarget)
{
	_ASSERT(!m_cancel);
	_ASSERT(m_coWalk.Finished());
	m_coWalk = WalkToPos(posTarget, m_cancel);
	m_coWalk.Run();//Э���뿪��ʼ���У����е���һ��co_await
}

bool ��Component::WalkToPos(Entity& refThis, const Position& posTarget)
{
	CHECK_RET_FALSE(refThis.m_up��);
	refThis.m_up��->WalkToPos(posTarget);
	return true;
}

CoTaskBool ��Component::WalkToPos(const Position posTarget, FunCancel& funCancel, const float f����Ŀ��С�ڴ˾���ͣ��)
{
	if (m_refEntity.IsDead())
	{
		//LOG(WARNING) << posTarget << ",������������";
		co_return false;
	}
	if (!m_refEntity.m_refSpace.CrowdTool��վ��(posTarget))
	{
		LOG(INFO) << posTarget << "����վ��";
		co_return false;
	}
	const auto posOld = m_refEntity.Pos();
	���λ����·���赲 _(m_refEntity);
	std::shared_ptr<RecastNavigationCrowd> sp;
	if (m_wpRecastNavigationCrowd.expired())
	{
		sp = std::make_shared<RecastNavigationCrowd, Entity&, const Position&>(m_refEntity, posTarget);
	}
	else
	{
		sp = m_wpRecastNavigationCrowd.lock();
		sp->SetMoveTarget(posTarget);
		//ref.SetSpeed();
	}


	m_wpRecastNavigationCrowd = sp;
	KeepCancel kc(funCancel);
	const auto posLocalTarget = posTarget;
	//m_refEntity.BroadcastChangeSkeleAnim("run");
	��λ::���λ���� ����;
	��λ::Find���λ����(m_refEntity.m_����, ����);
	EntitySystem::�ָ����ж��� __(m_refEntity, ����.str��·����);
	CoEvent<MyEvent::MoveEntity>::OnRecvEvent({ m_refEntity.weak_from_this() });
	while (true)
	{
		if (co_await CoTimer::WaitNextUpdate(funCancel))//�������������̴߳�ѭ����ÿ��ѭ������һ��
		{
			//LOG(INFO) << "����" << posLocalTarget << "��Э��ȡ����";
			co_return true;
		}
		if (m_refEntity.IsDead())
		{
			//LOG(INFO) << "�Լ�����������" << posLocalTarget << "��Э��ȡ����";
			//PlayerComponent::Say(m_refEntity, "�Լ�����", SayChannel::ϵͳ);

			co_return true;
		}

		if (���ߵ�Ŀ�긽��(m_refEntity, posLocalTarget, f����Ŀ��С�ڴ˾���ͣ��))
		{
			if (m_refEntity.m_upPlayer)
			{
				int a = 0;
			}

			for (int i = 0; i < 30; ++i)
			{
				auto wp��� = m_refEntity.Get�����Entity(Entity::�ѷ�, [](const Entity&) {return true; });
				if (wp���.expired())
					co_return false;

				Entity& ref��� = *wp���.lock();

				if (!m_refEntity.DistanceLessEqual(ref���, 2.0f))
				{
					//LOG(INFO) << "����û���ѷ���λ��ֹͣ����" << posLocalTarget;
					co_return false;
				}
				if (co_await CoTimer::WaitNextUpdate(funCancel))//�������������̴߳�ѭ����ÿ��ѭ������һ��
				{
					//LOG(INFO) << "���ߵ�Ŀ�긽��������" << posLocalTarget << "��Э��ȡ����";
					co_return true;
				}
			}

			LOG(INFO) << "���ߵ�Ŀ�긽�������������ѷ���λ," << posLocalTarget;
			co_return false;
		}

		//EntitySystem::BroadcastEntity����(refThis, std::format("��Ŀ��{0}��", (int)refThis.Pos().Distance(posTarget)));
	}
	LOG(INFO) << "����Ŀ��Э�̽���:" << posTarget;
	co_return false;
}

void ��Component::WalkToPos�ֶ�����(const Position& posTarget)
{
	//void CrowdToolSetMoveTarget(const float* p, const int idx);
	//float arrFloat[] = { posTarget.x,0,posTarget.z };
	//CrowdToolSetMoveTarget(arrFloat, refEntity.m_upAttack->m_idxCrowdAgent);
	//return;
	if (m_refEntity.IsDead())
	{
		//PlayerComponent::Say(m_refEntity, "�Լ�����,������", SayChannel::ϵͳ);

		return;
	}
	//m_coStop = true;

	OnEntityDestroy(false);

	m_coWalk.Run();
	_ASSERT(m_coWalk.Finished());//20240205
	//if (m_refEntity.m_upAttack)
	//{
	//	m_refEntity.m_upAttack->OnEntityDestroy(const bool bDestroy);
	//	//_ASSERT(m_refEntity.m_upAttack->m_coAttack.Finished());//20240205
	//}
	EntitySystem::ֹͣ����������(m_refEntity);
	/*m_coStop = false;*/
	_ASSERT(!m_cancel);
	m_coWalk�ֶ����� = WalkToPos(posTarget, m_cancel);
	m_coWalk�ֶ�����.Run();//Э���뿪��ʼ���У����е���һ��co_await
}


void ��Component::OnEntityDestroy(const bool bDestroy)
{
	if (m_cancel)
	{
		//LOG(INFO) << "����m_cancel";
		m_cancel.TryCancel();
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

void ��Component::�߽�(WpEntity wpEntity�ر�)
{
	//if (m_refEntity.m_upAttack)
	//	m_refEntity.m_upAttack->OnEntityDestroy(const bool bDestroy);

	//if (m_refEntity.m_up��Ŀ���߹�ȥ)
	//	m_refEntity.m_up��Ŀ���߹�ȥ->OnEntityDestroy(const bool bDestroy);

	//if (m_refEntity.m_upҽ�Ʊ�)
	//	m_refEntity.m_upҽ�Ʊ�->OnEntityDestroy(const bool bDestroy);
	EntitySystem::ֹͣ����������(m_refEntity);

	if (wpEntity�ر�.expired())
		return;

	if (wpEntity�ر�.lock()->m_upBuilding && !wpEntity�ر�.lock()->m_upBuilding->�����())
	{
		PlayerComponent::��������Buzz(m_refEntity, "�ر���û��ã����ܽ��ر�");
		return;
	}

	//if (�콨��Component::���ڽ���(m_refEntity))
	//{
	//	PlayerComponent::��������Buzz(m_refEntity, "���ڽ��죬���ܽ��ر�");
	//	return;
	//}

	//if (m_refEntity.m_upAttack)
	//	m_refEntity.m_upAttack->OnEntityDestroy(const bool bDestroy);
	EntitySystem::ֹͣ����������(m_refEntity);
	Cancel���а�����·��Э��(m_refEntity);
	_ASSERT(!m_cancel);
	_ASSERT(m_coWalk���ر�.Finished());
	m_coWalk���ر� = Co�߽��ر�(wpEntity�ر�);
	m_coWalk���ر�.Run();
}

CoTaskBool ��Component::Co�߽��ر�(WpEntity wpEntity�ر�)
{
	KeepCancel kc(m_cancel);

	if (m_refEntity.m_upAttack)
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
		if (!spEntity�ر�->m_up�ɽ����λ)
			co_return false;

		if (m_refEntity.DistanceLessEqual(*spEntity�ر�, m_refEntity.��������() + BuildingComponent::������߳�(*spEntity�ر�)))
		{
			spEntity�ر�->m_up�ɽ����λ->��(m_refEntity.m_refSpace, m_refEntity);

			co_return false;
		}

		_ASSERT(!m_cancel);
		if (co_await WalkToTarget(spEntity�ر�, m_cancel, false))
			co_return true;

	}

	co_return false;
}

void ��Component::Cancel���а�����·��Э��(Entity& refEntity, const bool bֹͣ����)
{
	if (bֹͣ����)EntitySystem::ֹͣ����������(refEntity);
	if (refEntity.m_up�ɼ�)		refEntity.m_up�ɼ�->m_TaskCancel.TryCancel();
	if (refEntity.m_up��)		refEntity.m_up��->OnEntityDestroy(false);
	if (refEntity.m_up�콨��)	refEntity.m_up�콨��->OnEntityDestroy(false);
}