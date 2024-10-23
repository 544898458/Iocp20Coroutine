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

�ɼ�Component::�ɼ�Component(Entity &refEntity) : m_Я��������(�����), m_refEntity(refEntity)
{
}

void �ɼ�Component::�ɼ�(PlayerGateSession_Game& refGateSession, WpEntity wpĿ����Դ)
{
	��Component::Cancel���а�����·��Э��(m_refEntity);
	m_TaskCancel.TryRun( Co�ɼ�(refGateSession, wpĿ����Դ));
}

CoTaskBool �ɼ�Component::Co�ɼ�(PlayerGateSession_Game& refGateSession, WpEntity wpĿ����Դ)
{
	using namespace std;
	while (true)
	{
		if (wpĿ����Դ.expired())
			co_return false;//Ŀ����Դû��

		auto spĿ����Դ = wpĿ����Դ.lock();
		auto sp��Դ = spĿ����Դ->m_sp��Դ;
		CHECK_CO_RET_FALSE(sp��Դ);
		

		auto wpEntity���� = m_refEntity.m_refSpace.Get�����Entity(m_refEntity, false,
			[](const Entity& ref)
			{
				return ref.m_spBuilding && ref.m_spBuilding->m_���� == ����;//�����Լ�����Ļ���
			});

		if (wpEntity����.expired())
		{
			if (co_await CoTimer::Wait(1s, m_TaskCancel.cancel))//�Լ���һ�����ض�û�У���һ�������
				co_return true;
			continue;
		}

		if (MaxЯ����() <= m_u32Я����)//װ���ˣ��ػ��طſ�
		{
			if (m_refEntity.DistanceLessEqual(*wpEntity����.lock(), m_refEntity.��������()))//�ڻ��ظ��������ؿ�ȫ���Ž����أ�ֱ�Ӽ�Ǯ��
			{
				if (co_await CoTimer::Wait(1s, m_TaskCancel.cancel))//�ѿ�Ž����غ�ʱ
					co_return true;

				auto addMoney = m_u32Я����;
				m_u32Я���� = 0;
				if (sp��Դ->m_���� == �����)
				{
					const auto& [stop, _] = co_await AiCo::ChangeMoney(refGateSession, addMoney, true, m_TaskCancel.cancel);
					if (stop)
						co_return true;

					CoEvent<MyEvent::��������˻ػ���>::OnRecvEvent(false, {});
				}
				else
				{
					refGateSession.m_u32ȼ���� += addMoney;
					refGateSession.Send��Դ();
				}
				continue;
			}

			//�����̫Զ���������
			m_refEntity.m_spAttack->TryCancel();
			if (co_await AiCo::WalkToTarget(m_refEntity, wpEntity����.lock(), m_TaskCancel.cancel, false))
				co_return true;//�жϣ����ܴ��ȥ��

			continue;
		}

		//��ûװ������Ҫ����ȥ�ɿ�
		
		if (m_refEntity.DistanceLessEqual(*spĿ����Դ, m_refEntity.��������()))//��Ŀ��󸽽�
		{
			CoEvent<MyEvent::��ʼ�ɼ������>::OnRecvEvent(false, {});
			if(co_await CoTimer::Wait(1s, m_TaskCancel.cancel))//�ɿ�1�����ʱ
				co_return true;//�ж�

			if (sp��Դ->m_���� != m_Я��������)
				m_u32Я����;

			++m_u32Я����;
			continue;
		}

		//����Ŀ���̫Զ���������
		//m_refEntity.m_spAttack->TryCancel();
		if (co_await AiCo::WalkToTarget(m_refEntity, wpĿ����Դ.lock(), m_TaskCancel.cancel, false))
			co_return true;//�ж�
	}
}

void �ɼ�Component::AddComponent(Entity& refEntity)
{
	refEntity.m_sp�ɼ� = std::make_shared<�ɼ�Component,Entity&>(refEntity);
}

