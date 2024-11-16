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

namespace ���˾���
{

	CoTask<int> Co(Space& refSpace, FunCancel& funCancel, PlayerGateSession_Game& refGateSession)
	{
		KeepCancel kc(funCancel);
		using namespace std;
		//PlayerGateSession_Game* pPlayerGateSession = nullptr;
		//{
		//	auto [stop, p] = co_await CoEvent<PlayerGateSession_Game*>::Wait(funCancel);
		//	if (stop)
		//		co_return 0;

		//	pPlayerGateSession = p;
		//}

		//CHECK_NOTNULL_CO_RET_0(pPlayerGateSession);
		refGateSession.Sayϵͳ("��ӭ������ʱ������Ϸ���˾���");
		if (co_await CoTimer::Wait(2s, funCancel))
			co_return 0;

		refGateSession.Sayϵͳ("�뵥��������ء���ť��3���������һ������");

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
		MonsterComponent::AddMonster(refSpace);

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
		MonsterComponent::AddMonster(refSpace, 10);

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
		co_return 0;
	}
}