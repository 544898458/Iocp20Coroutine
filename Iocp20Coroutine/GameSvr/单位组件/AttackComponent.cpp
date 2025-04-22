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
#include "��Ŀ���߹�ȥComponent.h"

extern std::unordered_map<int, uint64_t> m_mapEntityId;
AttackComponent& AttackComponent::AddComponent(Entity& refEntity)
{
	��Ŀ���߹�ȥComponent::AddComponent(refEntity);
	refEntity.AddComponentOnDestroy(&Entity::m_upAttack, new AttackComponent(refEntity));
	return *refEntity.m_upAttack;
}

using namespace std;
AttackComponent::AttackComponent(Entity& refEntity) :
	m_refEntity(refEntity)
{
	CHECK_RET_VOID(m_refEntity.m_up��Ŀ���߹�ȥ);

	m_refEntity.m_up��Ŀ���߹�ȥ->�����ѭ��(
		[this]()->bool {return this->���Թ���(); },
		[this]()->WpEntity {return Get����ĵ���(); },
		[this](const Entity& refTarget, WpEntity wpEntity, ��Ŀ���߹�ȥComponent& ref��Ŀ���߹�ȥ)->CoTask<std::tuple<bool, bool>> {return Co����(refTarget, wpEntity, ref��Ŀ���߹�ȥ); },
		[this](WpEntity& wpEntity, bool& ref���Ŀ��)->void{ this->������Ŀ��(wpEntity, ref���Ŀ��); }
	);
}

void AttackComponent::OnEntityDestroy(const bool bDestroy)
{
	if (m_cancelAttack)
	{
		//LOG(INFO) << "����m_cancel";
		m_cancelAttack();
		m_cancelAttack = nullptr;
	}
}

bool AttackComponent::���Թ���()
{
	if (m_refEntity.m_upBuilding && !m_refEntity.m_upBuilding->�����())
		return false;

	if (m_refEntity.IsDead())
		return false;

	if (�콨��Component::���ڽ���(m_refEntity))
		return false;

	if (m_refEntity.m_up��)
	{
		if (!m_refEntity.m_up��->m_coWalk�ֶ�����.Finished() ||
			!m_refEntity.m_up��->m_coWalk���ر�.Finished())
			return false;//��ʾ��������
	}
	return true;
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
		��Ŀ���߹�ȥComponent::����ǰҡ����(m_refEntity);

		CHECK_CO_RET_FALSE(m_refEntity.m_up��Ŀ���߹�ȥ);
		if (0s < m_refEntity.m_up��Ŀ���߹�ȥ->m_ս������.dura��ʼ���Ź������� && co_await CoTimer::Wait(m_refEntity.m_up��Ŀ���߹�ȥ->m_ս������.dura��ʼ���Ź�������, cancel))
			co_return true;//Э��ȡ��

		CHECK_��ֹ����Ŀ������;

		��Ŀ���߹�ȥComponent::���Ź�������(m_refEntity);
		const auto u16��ʼ�˺� = EntitySystem::�����󹥻�ǰҡ_�˺���ʱ(m_refEntity);
		if (0 < u16��ʼ�˺� && co_await CoTimer::Wait(std::chrono::milliseconds(u16��ʼ�˺�), cancel))
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
		CHECK_CO_RET_FALSE(m_refEntity.m_up��Ŀ���߹�ȥ);
		if (!m_refEntity.DistanceLessEqual(refDefencer, m_refEntity.m_up��Ŀ���߹�ȥ->��������(refDefencer)))
			break;//Ҫִ�к�ҡ

		if (!refDefencer.m_upDefence)
			break;//Ŀ�����

		��Ŀ���߹�ȥComponent::���Ź�����Ч(m_refEntity);

		const uint16_t u16������Ĺ��� = EntitySystem::�����󹥻�(m_refEntity);
		if (0 < u16������Ĺ���)
			refDefencer.m_upDefence->����(u16������Ĺ���, m_refEntity.Id);

		if (��ɫ̹�� == m_refEntity.m_����)
		{
			auto wp��� = m_refEntity.m_refSpace.����λ(std::forward<UpPlayerComponent>(m_refEntity.m_upPlayer), EntitySystem::GetNickName(m_refEntity), m_refEntity.Pos(), ���);
			_ASSERT(!wp���.expired());
			CHECK_WP_CO_RET_FALSE(wp���);
			auto& ref��� = *wp���.lock();
			CHECK_CO_RET_FALSE(m_refEntity.m_up��Ŀ���߹�ȥ);
			����Ŀ��Component::AddComponent(ref���, m_refEntity.Pos(), (refDefencer.Pos() - m_refEntity.Pos()).��һ��(), m_refEntity.m_up��Ŀ���߹�ȥ->m_ս������.f��������);
			_ASSERT(ref���.m_up����Ŀ��);
		}
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
		��Ŀ���߹�ȥComponent::����ǰҡ����(m_refEntity);
		EntitySystem::BroadcastEntity����(m_refEntity, "׼������");

		const auto u16��ʼ�˺� = EntitySystem::�����󹥻�ǰҡ_�˺���ʱ(m_refEntity);

		{
			SpEntity spEntity��Ч = std::make_shared<Entity, const Position&, Space&, ��λ����, const ��λ::��λ����&>(
				posTarget, m_refEntity.m_refSpace, ��Ч, { "ը��" ,��, "��Ч/ը��","" });
			m_refEntity.m_refSpace.AddEntity(spEntity��Ч, 0);
			spEntity��Ч->BroadcastEnter();
			CHECK_CO_RET_FALSE(m_refEntity.m_up��Ŀ���߹�ȥ);
			spEntity��Ч->DelayDelete(m_refEntity.m_up��Ŀ���߹�ȥ->m_ս������.dura��ʼ���Ź������� + std::chrono::milliseconds(u16��ʼ�˺�));
		}
		CHECK_CO_RET_FALSE(m_refEntity.m_up��Ŀ���߹�ȥ);
		if (0s < m_refEntity.m_up��Ŀ���߹�ȥ->m_ս������.dura��ʼ���Ź������� && co_await CoTimer::Wait(m_refEntity.m_up��Ŀ���߹�ȥ->m_ս������.dura��ʼ���Ź�������, cancel))
			co_return true;//Э��ȡ��

		CHECK_��ֹ����λ������;
		EntitySystem::BroadcastEntity����(m_refEntity, "");
		��Ŀ���߹�ȥComponent::���Ź�������(m_refEntity);
		if (0 < u16��ʼ�˺� && co_await CoTimer::Wait(std::chrono::milliseconds(u16��ʼ�˺�), cancel))
			co_return true;//Э��ȡ��

		CHECK_��ֹ����λ������;

		CHECK_CO_RET_FALSE(m_refEntity.m_up��Ŀ���߹�ȥ);
		if (!m_refEntity.Pos().DistanceLessEqual(posTarget, m_refEntity.m_up��Ŀ���߹�ȥ->��������(fĿ�꽨����߳�)))
			break;//Ҫִ�к�ҡ

		��Ŀ���߹�ȥComponent::���Ź�����Ч(m_refEntity);
		{
			SpEntity spEntity��Ч = std::make_shared<Entity, const Position&, Space&, ��λ����, const ��λ::��λ����&>(
				posTarget, m_refEntity.m_refSpace, ��Ч, { "��ը����" ,�� ,"��Ч/�ƹⱬ��","" });
			m_refEntity.m_refSpace.AddEntity(spEntity��Ч, 0);
			spEntity��Ч->BroadcastEnter();
			spEntity��Ч->DelayDelete(1s);
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

				if (refDefencer.m_upDefence && refDefencer.Pos().DistanceLessEqual(posTarget, 5))
					refDefencer.m_upDefence->����(EntitySystem::�����󹥻�(m_refEntity), m_refEntity.Id);
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

void AttackComponent::������Ŀ��(WpEntity& wpEntity, bool& ref���Ŀ��)
{
	//����б�
	if (m_refEntity.m_upDefence)
	{
		//�Ҷ����˺����ĵ���
		auto& refMap = m_refEntity.m_upDefence->m_map�����˺�;
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
			if(!EntitySystem::Is�յ��ܴ�(m_refEntity.m_����, refEntity.m_����))
			{
				refMap.erase(iterBegin);
				continue;
			}
			if (refEntity.IsDead())
			{
				refMap.erase(iterBegin);
				continue;
			}
			wpEntity = wp;
			ref���Ŀ�� = true;
			break;
		}
	}
}

CoTask<std::tuple<bool, bool>> AttackComponent::Co����(const Entity& refTarget, WpEntity wpEntity, ��Ŀ���߹�ȥComponent& ref��Ŀ���߹�ȥ)
{
	const bool b�����ѷ���λ̫�� = EntitySystem::�����ѷ���λ̫��(m_refEntity);

	if (!m_refEntity.DistanceLessEqual(refTarget, ref��Ŀ���߹�ȥ.��������(refTarget)) || b�����ѷ���λ̫�� || !ref��Ŀ���߹�ȥ.��鴩ǽ(refTarget))
		co_return{ false, false };

	��Component::Cancel���а�����·��Э��(m_refEntity); //TryCancel();

	if (m_refEntity.m_���� == ��ɫ̹��)
	{
		if (co_await CoAttackλ��(refTarget.Pos(), BuildingComponent::������߳�(refTarget), m_cancelAttack))
			co_return{ true, false };
	}
	else
	{
		if (co_await CoAttackĿ��(wpEntity, m_cancelAttack))
			co_return{ true, false };
	}

	co_return{ false, true };
}

WpEntity AttackComponent::Get����ĵ���()
{
	return m_refEntity.Get�����Entity֧�ֵر��еĵ�λ(Entity::�з�, [this](const Entity& ref)->bool
		{
			if (!EntitySystem::Is�յ��ܴ�(m_refEntity.m_����, ref.m_����))
				return false;
			if (!ref.m_upDefence)
				return false;
			return nullptr != ref.m_upDefence;
		});
}