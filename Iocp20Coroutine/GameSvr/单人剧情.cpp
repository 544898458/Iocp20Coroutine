#include "pch.h"
#include "���˾���.h"
#include "PlayerGateSession_Game.h"
#include "../CoRoutine/CoTimer.h"
#include "../CoRoutine/CoEvent.h"
#include "MyEvent.h"
#include "Entity.h"
#include "��ԴComponent.h"
#include "MonsterComponent.h"
#include "PlayerComponent.h"
#include "AttackComponent.h"
#include "����λComponent.h"
#include "��Component.h"
#include "��λ.h"
#include "AiCo.h"

namespace ���˾���
{

	CoTask<int> Coѵ��ս(Space& refSpace, FunCancel& funCancel, PlayerGateSession_Game& refGateSession)
	{
		KeepCancel kc(funCancel);
		using namespace std;
		refGateSession.Sayϵͳ("��ӭ������ʱ������Ϸ���˾���");
		if (co_await CoTimer::Wait(2s, funCancel))
			co_return 0;

		const ���λ���� ����(���λ����::���̳�);
		��λ::���λ���� ����;
		��λ::Find���λ����(����, ����);
		SpEntity sp���̳� = ����λComponent::����λ(refGateSession, { 30,30 }, ����, ����);

		refGateSession.Sayϵͳ("�뵥�������̳���ѡ�У�Ȼ�󵥻�������ء���ť���ٵ���հ׵��棬3���������һ������");

		const auto funSameSpace = [&refSpace, &refGateSession](const MyEvent::AddEntity& refAddEntity) { return MyEvent::SameSpace(refAddEntity.wpEntity, refSpace, refGateSession); };

		Position pos����;
		{
			const auto& [stop, addEvent] = co_await CoEvent<MyEvent::AddEntity>::Wait(funCancel, funSameSpace);
			if (stop)
				co_return 0;

			pos���� = addEvent.wpEntity.lock()->m_Pos;
		}

		refGateSession.Sayϵͳ("�뵥��ѡ�л��أ�����ɫ��Ч��ʾѡ�У���Ȼ�������칤�̳�����ť,2�����ڻ��������һ�����̳�");

		if (std::get<0>(co_await CoEvent<MyEvent::AddEntity>::Wait(funCancel, funSameSpace)))
			co_return 0;

		refGateSession.Sayϵͳ("��������һ�����̳��ˣ�����ѡ�����Ĺ��̳����ٵ�����棬����������Ŀ���");

		if (std::get<0>(co_await CoEvent<MyEvent::MoveEntity>::Wait(funCancel, [&refSpace, &refGateSession](const MyEvent::MoveEntity& ref)
			{
				if (ref.wpEntity.expired())
					return false;

				auto spEnity = ref.wpEntity.lock();
				if (&spEnity->m_refSpace != &refSpace)
					return false;

				if (!spEnity->m_spPlayer)
					return false;

				if (&spEnity->m_spPlayer->m_refSession != &refGateSession)
					return false;

				return true;
			})))
		{
			co_return 0;
		}

		refGateSession.Sayϵͳ("�ܺã����ڸ���ˢ��һ�����������������ù��̳��ھ����ͻ���֮����˾����");
		��ԴComponent::Add(refSpace, �����, { pos����.x,pos����.z - 20 });
		��ԴComponent::Add(refSpace, ȼ����, { pos����.x + 30,pos����.z });

		if (std::get<0>(co_await CoEvent<MyEvent::��ʼ�ɼ������>::Wait(funCancel)))
			co_return 0;

		refGateSession.Sayϵͳ("�ܺã����Ĺ��̳����ڲɼ������������Ѿ�����˻ػ���");

		if (std::get<0>(co_await CoEvent<MyEvent::��������˻ػ���>::Wait(funCancel)))
			co_return 0;

		refGateSession.Sayϵͳ("���Ĺ��̳��Ѱѵ�һ��������˵����أ���鿴���ϽǾ���������仯");

		if (co_await CoTimer::Wait(3s, funCancel))
			co_return 0;

		refGateSession.Sayϵͳ("�뵥�������������ť");
		if (std::get<0>(co_await CoEvent<MyEvent::AddEntity>::Wait(funCancel, funSameSpace)))
			co_return 0;

		refGateSession.Sayϵͳ("�뵥��ѡ�б���������ɫ��Ч��ʾѡ�У���Ȼ�������������ť,2�����ڱ��������һ����");

		if (std::get<0>(co_await CoEvent<MyEvent::AddEntity>::Wait(funCancel, funSameSpace)))
			co_return 0;

		refGateSession.Sayϵͳ("��굥�����ı����ٵ�����棬����ָ��������Ŀ�괦");

		if (std::get<0>(co_await CoEvent<MyEvent::MoveEntity>::Wait(funCancel, [&refSpace](const MyEvent::MoveEntity& ref) {return &ref.wpEntity.lock()->m_refSpace == &refSpace; })))
			co_return 0;

		refGateSession.Sayϵͳ("�����Ѹ���ˢ��һ���֣����Ʊ��ߵ��ָ����������Զ����");
		MonsterComponent::AddMonster(refSpace, { -30.0 });

		if (std::get<0>(co_await CoEvent<MyEvent::��λ����>::Wait(funCancel, [&refSpace](const MyEvent::��λ����& ref) {return &ref.wpEntity.lock()->m_refSpace == &refSpace; })))
			co_return 0;

		refGateSession.Sayϵͳ("���ı������ˡ����������ȥΧ������");

		if (std::get<0>(co_await CoEvent<MyEvent::��λ����>::Wait(funCancel, [&refSpace](const MyEvent::��λ����& ref)
			{
				auto spEntity = ref.wpEntity.lock();
				if (&spEntity->m_refSpace != &refSpace)
					return false;

				return !spEntity->m_spPlayer;//��������
			})))
		{
			co_return 0;
		}

		refGateSession.Sayϵͳ("��ϲ�������˵��ˣ����ڸ���ˢ��10�����ˡ���������ر�,�ñ�����ر��У�������أ����Ż�����");
		MonsterComponent::AddMonster(refSpace, { -30.0 }, 10);

		if (std::get<0>(co_await CoEvent<MyEvent::��λ����>::Wait(funCancel, [&refSpace](const MyEvent::��λ����& ref)
			{
				auto spEntity = ref.wpEntity.lock();
				if (&spEntity->m_refSpace != &refSpace)
					return false;

				for (const auto [k, v] : refSpace.m_mapEntity)
				{
					if (v->IsDead())
						continue;

					if (nullptr == v->m_spMonster)
						continue;

					if (nullptr == v->m_spPlayer)
						return false;//���йֻ���
				}

				return true;
			})))
		{
			co_return 0;
		}

		refGateSession.Sayϵͳ("��ȡ����ʤ��������ָ����ţ�");
		refGateSession.Send<Msg��ʾ����>({ .ui = Msg��ʾ����::ѡ���ͼ });
		co_return 0;
	}
	Position �����й�(const Position& refOld)
	{
		return { -30,30 };
	}
	CoTask<int> Co����ս(Space& refSpace, FunCancel& funCancel, PlayerGateSession_Game& refGateSession)
	{
		KeepCancel kc(funCancel);
		using namespace std;
		refGateSession.Sayϵͳ("����ս��ֻҪ��ס������ʤ����");
		if (co_await CoTimer::Wait(1s, funCancel))
			co_return 0;

		//co_await AiCo::ChangeMoney(refGateSession, 1000, true, funCancel);
		{
			const ���λ���� ����(���λ����::���̳�);
			��λ::���λ���� ����;
			��λ::Find���λ����(����, ����);
			SpEntity sp���̳� = ����λComponent::����λ(refGateSession, { -30, 30 }, ����, ����);

			��ԴComponent::Add(refSpace, �����, { -20, 30 });
			��ԴComponent::Add(refSpace, �����, { -25, 25});
			��ԴComponent::Add(refSpace, ȼ����, { -25, 30 });
			��ԴComponent::Add(refSpace, ȼ����, { -20, 25 });

		}
		for (int i = 0; i < 15; ++i)
		{
			if (co_await CoTimer::Wait(20s, funCancel))
				co_return 0;

			auto vecEneity = MonsterComponent::AddMonster(refSpace, { 48,-48 }, i);
			for (auto& spEntity : vecEneity)
			{
				//if (spEntity->m_sp��)
					//spEntity->m_sp��->WalkToPos({ -30, 30 });

				spEntity->m_spAttack->m_fun��������˴� = �����й�;
			}
		}

		//while (true)
		//{
		//	if (co_await CoTimer::Wait(5s, funCancel))
		//		co_return 0;

		//	for (auto [id, spEntity] : refSpace.m_mapEntity)
		//	{
		//		if (spEntity->m_spPlayer)
		//			continue;

		//		if (spEntity->m_sp��)//�ÿ��еĹ�����Ŀ��
		//		{
		//			��Component::Cancel���а�����·��Э��(*spEntity); //TryCancel();
		//			spEntity->m_sp��->WalkToPos({ -30, 30 });
		//		}
		//	}
		//}

		co_return 0;
	}
}