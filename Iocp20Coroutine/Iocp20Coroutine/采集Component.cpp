#include "pch.h"
#include "�ɼ�Component.h"
#include "BuildingComponent.h"
#include "Entity.h"
#include "../CoRoutine/CoTimer.h"
#include "../CoRoutine/CoEvent.h"
#include "Space.h"
#include "AiCo.h"
#include "MyEvent.h"

void �ɼ�Component::�ɼ�(PlayerGateSession_Game& refGateSession, Entity& refThis, WpEntity wpĿ����Դ)
{
	Co�ɼ�(refGateSession, refThis, wpĿ����Դ).RunNew();
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
			co_await CoTimer::Wait(1s, m_funCancel);//�Լ���һ�����ض�û�У���һ�������
			continue;
		}

		if (MaxЯ�������() <= m_u32Я�������)//װ���ˣ��ػ��طſ�
		{
			if (refThis.DistanceLessEqual(*wpEntity����.lock(), refThis.m_f��������))//�ڻ��ظ��������ؿ�ȫ���Ž����أ�ֱ�Ӽ�Ǯ��
			{
				if (co_await CoTimer::Wait(1s, m_funCancel))//�ѿ�Ž����غ�ʱ
					co_return true;

				auto addMoney = m_u32Я�������;
				m_u32Я������� = 0;
				const auto& [stop, _] = co_await AiCo::ChangeMoney(refGateSession, addMoney, true, m_funCancel);
				if (stop)
					co_return true;

				CoEvent<MyEvent::��������˻ػ���>::OnRecvEvent(false, {});
				continue;
			}

			//�����̫Զ���������
			if (co_await AiCo::WalkToTarget(refThis.shared_from_this(), wpEntity����.lock(), m_funCancel, false))
				co_return true;

			continue;
		}

		//��ûװ������Ҫ����ȥ�ɿ�
		if (refThis.DistanceLessEqual(*wpĿ����Դ.lock(), refThis.m_f��������))//��Ŀ��󸽽�
		{
			CoEvent<MyEvent::��ʼ�ɼ������>::OnRecvEvent(false, {});
			co_await CoTimer::Wait(1s, m_funCancel);//�ɿ�1�����ʱ
			++m_u32Я�������;
			continue;
		}

		//����Ŀ���̫Զ���������
		if (co_await AiCo::WalkToTarget(refThis.shared_from_this(), wpĿ����Դ.lock(), m_funCancel, false))
			co_return true;
	}
}

void �ɼ�Component::AddComponent(Entity& refEntity)
{
	refEntity.m_sp�ɼ� = std::make_shared<�ɼ�Component>();
}
