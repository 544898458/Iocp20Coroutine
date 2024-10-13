#include "pch.h"
#include "�ɼ�Component.h"
#include "BuildingComponent.h"
#include "Entity.h"
#include "../CoRoutine/CoTimer.h"
#include "Space.h"
#include "AiCo.h"

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
		{
			co_return 0;
		}

		auto wpEntity���� = refThis.m_refSpace.Get�����Entity(refThis, false, [](const Entity& ref)//�����Լ�����Ļ���
			{
				if (!ref.m_spBuilding)
					return false;

				return ref.m_spBuilding->m_���� == ����;
			});

		if (wpEntity����.expired())
		{
			co_await CoTimer::Wait(1s, m_funCancel);//û�л��أ���һ�������
			continue;
		}

		if (MaxЯ�������() <= m_u32Я�������)//�ػ��طſ�
		{
			//���������
			if (refThis.DistanceLessEqual(*wpEntity����.lock(), 3))//�ڻ��ظ��������ؿ�ȫ���Ž����أ�ֱ�Ӽ�Ǯ��
			{
				const auto& [stop, _] = co_await AiCo::ChangeMoney(refGateSession, m_u32Я�������, true, m_funCancel);
				if (stop)
				{
					co_return 0;
				}

				m_u32Я������� = 0;
				if(co_await CoTimer::Wait(1s, m_funCancel))//�ɿ��ٶ�
				{
					co_return 0;
				}
				continue;
			}
			else
			{
				//�������
				if (co_await AiCo::WalkToTarget(refThis.shared_from_this(), wpEntity����.lock(), m_funCancel))
				{
					co_return 0;
				}
				continue;
			}
		}
		else//��Ҫ����ȥ�ɿ�
		{
			if (refThis.DistanceLessEqual(*wpĿ����Դ.lock(), 3))
			{
				++m_u32Я�������;
				co_await CoTimer::Wait(1s, m_funCancel);//�ɿ��ٶ�
				continue;
			}
			else
			{
				//�������
				co_await AiCo::WalkToTarget(refThis.shared_from_this(), wpĿ����Դ.lock(), m_funCancel);
				continue;
			}
		}
	}
}

void �ɼ�Component::AddComponent(Entity& refEntity)
{
	refEntity.m_sp�ɼ� = std::make_shared<�ɼ�Component>();
}
