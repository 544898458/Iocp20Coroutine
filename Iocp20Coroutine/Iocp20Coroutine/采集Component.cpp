#include "pch.h"
#include "�ɼ�Component.h"
#include "BuildingComponent.h"
#include "Entity.h"
#include "../CoRoutine/CoTimer.h"
#include "../CoRoutine/CoEvent.h"
#include "Space.h"
#include "AiCo.h"
#include "MyEvent.h"
#include "AttackComponent.h"

void �ɼ�Component::�ɼ�(PlayerGateSession_Game& refGateSession, Entity& refThis, WpEntity wpĿ����Դ)
{
	m_TaskCancel.TryRun( Co�ɼ�(refGateSession, refThis, wpĿ����Դ));
}

CoTaskBool �ɼ�Component::Co�ɼ�(PlayerGateSession_Game& refGateSession, Entity& refThis, WpEntity wpĿ����Դ)
{
	using namespace std;
	while (true)
	{
		if (wpĿ����Դ.expired())
			co_return false;//Ŀ����Դû��

		auto wpEntity���� = refThis.m_refSpace.Get�����Entity(refThis, false,
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

		if (MaxЯ�������() <= m_u32Я�������)//װ���ˣ��ػ��طſ�
		{
			if (refThis.DistanceLessEqual(*wpEntity����.lock(), refThis.m_f��������))//�ڻ��ظ��������ؿ�ȫ���Ž����أ�ֱ�Ӽ�Ǯ��
			{
				if (co_await CoTimer::Wait(1s, m_TaskCancel.cancel))//�ѿ�Ž����غ�ʱ
					co_return true;

				auto addMoney = m_u32Я�������;
				m_u32Я������� = 0;
				const auto& [stop, _] = co_await AiCo::ChangeMoney(refGateSession, addMoney, true, m_TaskCancel.cancel);
				if (stop)
					co_return true;

				CoEvent<MyEvent::��������˻ػ���>::OnRecvEvent(false, {});
				continue;
			}

			//�����̫Զ���������
			refThis.m_spAttack->TryCancel();
			if (co_await AiCo::WalkToTarget(refThis.shared_from_this(), wpEntity����.lock(), m_TaskCancel.cancel, false))
				co_return true;//�жϣ����ܴ��ȥ��

			continue;
		}

		//��ûװ������Ҫ����ȥ�ɿ�
		if (refThis.DistanceLessEqual(*wpĿ����Դ.lock(), refThis.m_f��������))//��Ŀ��󸽽�
		{
			CoEvent<MyEvent::��ʼ�ɼ������>::OnRecvEvent(false, {});
			if(co_await CoTimer::Wait(1s, m_TaskCancel.cancel))//�ɿ�1�����ʱ
				co_return true;//�ж�
			++m_u32Я�������;
			continue;
		}

		//����Ŀ���̫Զ���������
		refThis.m_spAttack->TryCancel();
		if (co_await AiCo::WalkToTarget(refThis.shared_from_this(), wpĿ����Դ.lock(), m_TaskCancel.cancel, false))
			co_return true;//�ж�
	}
}

void �ɼ�Component::AddComponent(Entity& refEntity)
{
	refEntity.m_sp�ɼ� = std::make_shared<�ɼ�Component>();
}
