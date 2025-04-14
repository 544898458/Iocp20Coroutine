#include "pch.h"
#include "�ɼ�Component.h"
#include "��ԴComponent.h"
#include "BuildingComponent.h"
#include "Entity.h"
#include "../CoRoutine/CoTimer.h"
#include "../CoRoutine/CoEvent.h"
#include "Space.h"
#include "AiCo.h"
#include "MyEvent.h"
#include "AttackComponent.h"
#include "PlayerGateSession_Game.h"
#include "��Component.h"
#include "EntitySystem.h"
#include "PlayerComponent.h"
#include "�콨��Component.h"
#include "��λ.h"
#include "PlayerNickNameComponent.h"

�ɼ�Component::�ɼ�Component(Entity& refEntity) : m_Я��������(�����), m_refEntity(refEntity), m_Ŀ����Դ����(��λ����_Invalid_0)
{
}

void �ɼ�Component::�ɼ�(WpEntity wpĿ����Դ)
{
	if (�콨��Component::���ڽ���(m_refEntity))
	{
		PlayerComponent::��������Buzz(m_refEntity, "���ڽ��죬���ܲɼ�");
		return;
	}
	��Component::Cancel���а�����·��Э��(m_refEntity);
	PlayerComponent::��������(m_refEntity, "����/����Ů���ɰ���");
	m_TaskCancel.TryRun(Co�ɼ�(wpĿ����Դ));
}

bool ��Դ�ɲɼ�(WpEntity& refWpĿ����Դ)
{
	if (refWpĿ����Դ.expired())
		return false;

	auto sp��Դ = refWpĿ����Դ.lock()->m_sp��Դ;
	CHECK_FALSE(sp��Դ);
	if (sp��Դ->m_�ɲɼ����� <= 0)
	{
		refWpĿ����Դ.lock()->CoDelayDelete().RunNew();
		return false;
	}

	return true;
}
std::tuple<SpEntity, std::shared_ptr<��ԴComponent>> �ɼ�Component::GetĿ����Դ(WpEntity& refWpĿ����Դ)
{
	if (!��Դ�ɲɼ�(refWpĿ����Դ))
	{
		if (��λ����::��λ����_Invalid_0 == m_Ŀ����Դ����)
			return {};

		auto wp��� = m_refEntity.Get�����Entity(Entity::����, m_Ŀ����Դ����);
		if (wp���.expired())
			return {};

		if (!m_refEntity.DistanceLessEqual(*wp���.lock(), 35))
			return {};

		refWpĿ����Դ = wp���;
	}

	auto spĿ����Դ = refWpĿ����Դ.lock();
	auto sp��Դ = spĿ����Դ->m_sp��Դ;
	m_Ŀ����Դ���� = sp��Դ->m_����;
	return { spĿ����Դ ,sp��Դ };
}

CoTaskBool �ɼ�Component::Co�ɼ�(WpEntity wpĿ����Դ)
{
	using namespace std;
	while (true)
	{
		//auto wpEntity���� = m_refEntity.m_refSpace.Get�����Entity֧�ֵر��еĵ�λ(m_refEntity, false,
		//	[](const Entity& ref)
		//	{
		//		return ref.m_spBuilding && ref.m_spBuilding->m_���� == ����;//�����Լ�����Ļ���
		//	});
		if (m_refEntity.IsDead())
		{
			LOG(INFO) << "�ɼ��������Լ�����";
			co_return false;
		}

		if (!m_refEntity.m_spPlayerNickName)
		{
			LOG(WARNING) << "�ɼ���������Ҷ���";
			co_return false;
		}

		float min�����ƽ�� = std::numeric_limits<float>::max();
		WpEntity wpEntity����;
		for (const auto [id, wp] : m_refEntity.m_refSpace.m_mapPlayer[m_refEntity.m_spPlayerNickName->m_strNickName].m_mapWpEntity)
		{
			CHECK_WP_CONTINUE(wp);
			const auto& refEntity = *wp.lock();
			if (!refEntity.m_spBuilding || (refEntity.m_���� != ���� && refEntity.m_���� != �泲))//�����Լ�����Ļ���
				continue;

			const auto pow2 = m_refEntity.DistancePow2(refEntity);
			if (min�����ƽ�� <= pow2)
				continue;

			min�����ƽ�� = pow2;
			wpEntity���� = wp;
		}
		if (wpEntity����.expired())
		{
			//if (co_await CoTimer::Wait(1s, m_TaskCancel.cancel))//�Լ���һ�����ض�û��
			//	co_return true;

			co_return false;
		}
		if (MaxЯ����() <= m_u32Я����)//װ���ˣ��ػ��طſ�
		{
			if (m_refEntity.DistanceLessEqual(*wpEntity����.lock(), m_refEntity.��������() + BuildingComponent::������߳�(*wpEntity����.lock())))//�ڻ��ظ��������ؿ�ȫ���Ž����أ�ֱ�Ӽ�Ǯ��
			{
				EntitySystem::BroadcastEntity����(m_refEntity, "����ж��");
				if (co_await CoTimer::Wait(2s, m_TaskCancel.cancel))//�ѿ�Ž����غ�ʱ
					co_return true;

				const auto u32Я���� = m_u32Я����;
				m_u32Я���� = 0;
				auto [_, sp��Դ] = GetĿ����Դ(wpĿ����Դ);
				if (!sp��Դ)
				{
					������Դ�ݽ�();
					co_return false;//Ŀ����Դû��
				}

				if (sp��Դ->m_���� == �����)
				{
					//const auto& [stop, _] = co_await AiCo::ChangeMoney(refGateSession, addMoney, true, m_TaskCancel.cancel);
					//if (stop)
					//	co_return true;
					Space::GetSpacePlayer(m_refEntity).m_u32����� += u32Я����;
					CoEvent<MyEvent::��������˻ػ���>::OnRecvEvent({});
				}
				else
				{
					Space::GetSpacePlayer(m_refEntity).m_u32ȼ���� += u32Я����;
				}
				PlayerComponent::Send��Դ(m_refEntity);
				continue;
			}

			//�����̫Զ���������
			//m_refEntity.m_spAttack->TryCancel();
			EntitySystem::ֹͣ����������(m_refEntity);
			if (co_await AiCo::WalkToTarget(m_refEntity, wpEntity����.lock(), m_TaskCancel.cancel, false))
				co_return true;//�жϣ����ܴ��ȥ��

			continue;
		}

		//��ûװ������Ҫ����ȥ�ɿ�
		{
			{
				auto [spEntity��Դ, _] = GetĿ����Դ(wpĿ����Դ);
				if (!spEntity��Դ)
				{
					������Դ�ݽ�();
					co_return false;//Ŀ����Դû��
				}

				if (!m_refEntity.DistanceLessEqual(*spEntity��Դ, m_refEntity.��������()))
				{
					EntitySystem::BroadcastEntity����(m_refEntity, std::format("����{0}", spEntity��Դ->m_���� == ����� ? "�����" : "ȼ����"));
					//m_refEntity.m_spAttack->TryCancel();
					if (co_await AiCo::WalkToTarget(m_refEntity, wpĿ����Դ.lock(), m_TaskCancel.cancel, false))
						co_return true;//�ж�

					m_refEntity.BroadcastChangeSkeleAnim(m_refEntity.m_���� == ���� ? "�ɼ�" : "2", true);
				}
			}

			//��Ŀ��󸽽�
			CoEvent<MyEvent::��ʼ�ɼ������>::OnRecvEvent({});
			if (co_await CoTimer::Wait(1s, m_TaskCancel.cancel))//�ɿ�1�����ʱ
				co_return true;//�ж�

			auto [spEntity��Դ, sp��Դ] = GetĿ����Դ(wpĿ����Դ);
			if (!spEntity��Դ || !sp��Դ)
			{
				������Դ�ݽ�();
				co_return false;//Ŀ����Դ�Ѳɿ�
			}

			if (sp��Դ->m_���� != m_Я��������)
			{
				m_u32Я���� = 0;
				m_Я�������� = sp��Դ->m_����;
			}
			if (0 == m_u32Я����)
			{
				switch (sp��Դ->m_����)
				{
				case �����:PlayerComponent::��������(m_refEntity, "TSCMin00"); break;
				case ȼ����:PlayerComponent::��������(m_refEntity, "TSCMin01"); break;
				default:break;
				}
			}

			--sp��Դ->m_�ɲɼ�����;
			EntitySystem::BroadcastEntity����(*spEntity��Դ, std::format("ʣ��:{0}", sp��Դ->m_�ɲɼ�����));

			++m_u32Я����;
			EntitySystem::BroadcastEntity����(m_refEntity, std::format("�Ѳɼ�{0}", m_u32Я����));
			continue;
		}
	}
}

void �ɼ�Component::������Դ�ݽ�()
{
	if (����� == m_Я��������)
		PlayerComponent::��������(m_refEntity, m_refEntity.m_���� == ���̳� ? "����/������ѿݽ�Ů���ɰ���" : "����/������ѿݽ�_����", "����� �ѿݽ�");
	else
		PlayerComponent::��������(m_refEntity, m_refEntity.m_���� == ���̳� ? "����/ȼ�����ѿݽ�Ů���ɰ���" : "����/ȼ�����ѿݽ�_����", "ȼ���� �ѿݽ�");

}
void �ɼ�Component::AddComponent(Entity& refEntity)
{
	refEntity.m_sp�ɼ� = std::make_shared<�ɼ�Component, Entity&>(refEntity);
}

bool �ɼ�Component::���ڲɼ�(Entity& refEntity)
{
	if (!refEntity.m_sp�ɼ�)
		return false;

	return !refEntity.m_sp�ɼ�->m_TaskCancel.co.Finished();
}

