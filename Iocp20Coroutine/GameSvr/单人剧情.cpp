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
		refGateSession.Sayϵͳ("��ӭ����RTS��ʱս����Ϸ��������Ҫ���ܻ�����ѵ��");

		auto [stop, msgResponce] = co_await AiCo::ChangeMoney(refGateSession, 100, true, funCancel);
		if (stop)
		{
			LOG(WARNING) << "ChangeMoney,Э��ȡ��";
			co_return 0;
		}

		if (co_await CoTimer::Wait(2s, funCancel))
			co_return 0;

		const ���λ���� ����(���λ����::���̳�);
		��λ::���λ���� ����;
		��λ::Find���λ����(����, ����);
		SpEntity sp���̳� = ����λComponent::����λ(refGateSession, { 5,10 }, ����, ����);

		refGateSession.Sayϵͳ("���������̳���ѡ�У�Ȼ����������ء���ť���ٵ���հ׵��棬10���������һ������");

		const auto funSameSpace = [&refSpace, &refGateSession](const MyEvent::AddEntity& refAddEntity) { return MyEvent::SameSpace(refAddEntity.wpEntity, refSpace, refGateSession); };

		Position pos����;
		{
			const auto& [stop, addEvent] = co_await CoEvent<MyEvent::AddEntity>::Wait(funCancel, funSameSpace);
			if (stop)
				co_return 0;

			pos���� = addEvent.wpEntity.lock()->Pos();
		}


		if (co_await CoTimer::Wait(10s, funCancel))
			co_return 0;


		refGateSession.Sayϵͳ("����ѡ�л��أ�Բ����Ч��ʾѡ�У���Ȼ�������칤�̳�����ť��5�����ڻ��������һ�����̳�");

		if (std::get<0>(co_await CoEvent<MyEvent::AddEntity>::Wait(funCancel, funSameSpace)))
			co_return 0;

		refGateSession.Sayϵͳ("�������й��̳��ˣ����ѡ�����Ĺ��̳����ٵ���տ����棬����������Ŀ���");

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
		��ԴComponent::Add(refSpace, ȼ����, { pos����.x + 15,pos����.z });

		if (std::get<0>(co_await CoEvent<MyEvent::��ʼ�ɼ������>::Wait(funCancel)))
			co_return 0;

		refGateSession.Sayϵͳ("�ܺã����Ĺ��̳����ڲɼ������������Ѿ�����˻ػ��ء����ڣ���ѡ����һ�����̳�ȥ�ɼ�ȼ����");
		if (std::get<0>(co_await CoEvent<MyEvent::��������˻ػ���>::Wait(funCancel)))
			co_return 0;

		refGateSession.Sayϵͳ("���Ĺ��̳��Ѱѵ�һ��������˵����أ���鿴���ϽǾ���������仯");

		if (co_await CoTimer::Wait(5s, funCancel))
			co_return 0;

		refGateSession.Sayϵͳ("�����湻20��������ѡ��һ�����̳���Ȼ���������񷿡���ť���ٵ��һ�οտ�����");
		if (std::get<0>(co_await CoEvent<MyEvent::AddEntity>::Wait(funCancel, funSameSpace)))
			co_return 0;

		refGateSession.Sayϵͳ("�ܺã��񷿿����������Ļ��λ��������");

		if (co_await CoTimer::Wait(3s, funCancel))
			co_return 0;

		refGateSession.Sayϵͳ("�����湻30��������ѡ��һ�����̳���Ȼ���������������ť���ٵ��һ�οտ����棬�������һ������");
		if (std::get<0>(co_await CoEvent<MyEvent::AddEntity>::Wait(funCancel, funSameSpace)))
			co_return 0;

		if (co_await CoTimer::Wait(10s, funCancel))
			co_return 0;

		refGateSession.Sayϵͳ("����ѡ�б�����Բ����Ч��ʾѡ�У���Ȼ�������������ť,10�����ڱ��������һ����");

		if (std::get<0>(co_await CoEvent<MyEvent::AddEntity>::Wait(funCancel, funSameSpace)))
			co_return 0;

		refGateSession.Sayϵͳ("�����ѡ�����ı����ٵ�����棬����ָ��������Ŀ�괦");

		if (std::get<0>(co_await CoEvent<MyEvent::MoveEntity>::Wait(funCancel, [&refSpace](const MyEvent::MoveEntity& ref) {return &ref.wpEntity.lock()->m_refSpace == &refSpace; })))
			co_return 0;

		refGateSession.Sayϵͳ("����������߸���ˢ��һ���֣����Ʊ��ߵ��ָ����������Զ���֡������Ե����½ǡ�ȡ��ѡ�С�Ȼ���϶����濴����������");
		MonsterComponent::AddMonster(refSpace, ��, { -30,20 });

		if (std::get<0>(co_await CoEvent<MyEvent::��λ����>::Wait(funCancel, [&refSpace](const MyEvent::��λ����& ref) {return &ref.wpEntity.lock()->m_refSpace == &refSpace; })))
			co_return 0;

		refGateSession.Sayϵͳ("���ı������ˡ����Զ�����ȥΧ������");

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

		refGateSession.Sayϵͳ("��ϲ�������˵��ˣ�������߸���ˢ��10�����ˡ���������ر�,�ñ�����ر��У�������أ����Ż�����");
		MonsterComponent::AddMonster(refSpace, ��, { -30.0 }, 10);

		if (std::get<0>(co_await CoEvent<MyEvent::��λ����>::Wait(funCancel, [&refSpace](const MyEvent::��λ����& ref)
			{
				auto spEntity = ref.wpEntity.lock();
				if (&spEntity->m_refSpace != &refSpace)
					return false;

				return 0 == refSpace.Get���ﵥλ��();
			})))
		{
			co_return 0;
		}

		refGateSession.��������("��Ч/YouWin", "��ȡ����ʤ����������ָ����ţ�");

		refGateSession.Send<Msg��ʾ����>({ .ui = Msg��ʾ����::ѡ���ͼ });
		co_return 0;
	}
	Position �����й�(const Position& refOld)
	{
		return { -30,30 };
	}
	bool Isս������(Space& refSpace, PlayerGateSession_Game& refGateSession)
	{
		if (0 == refSpace.Get���ﵥλ��())
		{
			refGateSession.��������("��Ч/YouWin", "����ס�ˣ�������ָ����ţ�");
			return true;
		}
		if (0 == refSpace.Get��ҵ�λ��(refGateSession))
		{
			refGateSession.��������("��Ч/YouLose", "ʤ���˱��ҳ��£��������Ͻǡ��˳��������뿪��Ȼ���ٴε��������ս�����Ϳ�������������");
			return true;
		}
		return false;
	}
	CoTask<int> Co����ս(Space& refSpace, FunCancel& funCancel, PlayerGateSession_Game& refGateSession)
	{
		KeepCancel kc(funCancel);
		using namespace std;
		refGateSession.Sayϵͳ("����ս��ֻҪ��ס������ʤ����");
		if (co_await CoTimer::Wait(1s, funCancel))
			co_return 0;

		refGateSession.m_u32ȼ���� += 20;
		{
			refGateSession.m_u32ȼ���� += 200;
			auto [stop, msgResponce] = co_await AiCo::ChangeMoney(refGateSession, 1000, true, funCancel);
			if (stop)
			{
				LOG(WARNING) << "ChangeMoney,Э��ȡ��";
				co_return 0;
			}
		}
		auto [stop, msgResponce] = co_await AiCo::ChangeMoney(refGateSession, 100, true, funCancel);
		if (stop)
		{
			LOG(WARNING) << "ChangeMoney,Э��ȡ��";
			co_return 0;
		}
		{
			{
				const ���λ���� ����(���λ����::���̳�);
				��λ::���λ���� ����;
				��λ::Find���λ����(����, ����);
				����λComponent::����λ(refGateSession, { -30, 30 }, ����, ����);
			}

			��ԴComponent::Add(refSpace, �����, { -20, 35 });
			��ԴComponent::Add(refSpace, �����, { -26, 20 });
			��ԴComponent::Add(refSpace, ȼ����, { -29, 35 });
			��ԴComponent::Add(refSpace, ȼ����, { -20, 20 });

		}

		refGateSession.Sayϵͳ("5��󽫳��ֵ�1���֡��������켸���ر����ñ���פ�ر�");
		if (co_await CoTimer::Wait(5s, funCancel))
			co_return 0;

		for (int i = 1; i < 8; ++i)
		{
			if (1 < i && Isս������(refSpace, refGateSession))
				co_return 0;

			refGateSession.Sayϵͳ(std::format("��{0}����������������", i));
			auto vecEneity = MonsterComponent::AddMonster(refSpace, i % 2 == 0 ? �� : ��ս��, { 48,-48 }, i * 20);
			for (auto& spEntity : vecEneity)
			{
				//if (spEntity->m_sp��)
					//spEntity->m_sp��->WalkToPos({ -30, 30 });

				spEntity->m_spAttack->m_fun��������˴� = �����й�;
			}

			if (co_await CoTimer::Wait(20s, funCancel))
				co_return 0;
		}

		//if (std::get<0>(co_await CoEvent<MyEvent::��λ����>::Wait(funCancel, [&refSpace](const MyEvent::��λ����& ref)
		//	{
		//		auto spEntity = ref.wpEntity.lock();
		//		if (&spEntity->m_refSpace != &refSpace)
		//			return false;

		//		return 0 == refSpace.Get���ﵥλ��();
		//	})))
		//{
		//	co_return 0;
		//}
		while (!co_await CoTimer::Wait(5s, funCancel))
		{
			if (Isս������)
				co_return 0;
		}
		co_return 0;
	}
}