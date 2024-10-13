#include "pch.h"
#include "���˾���.h"
#include "PlayerGateSession_Game.h"
#include "../CoRoutine/CoTimer.h"
#include "../CoRoutine/CoEvent.h"
#include "MyEvent.h"
#include "Entity.h"
#include "��ԴComponent.h"

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
		refGateSession.Say("��ӭ������ʱ������Ϸ���˾���");
		if (co_await CoTimer::Wait(1s, funCancel))
			co_return 0;

		refGateSession.Say("�뵥��������ء���ť��3���������һ������");


		auto funSameSpace = [&refSpace](const WpEntity& refWpEntity) { return MyEvent::SameSpace(refWpEntity, refSpace); };
		{
			auto [stop, wpEntity] = co_await CoEvent<WpEntity>::Wait(funCancel, funSameSpace);
			if (stop)co_return 0;
		}

		refGateSession.Say("�뵥��ѡ�л��أ�����ɫ��Ч��ʾѡ�У���Ȼ�������������ť");

		{
			auto [stop, wpEntity] = co_await CoEvent<WpEntity>::Wait(funCancel, funSameSpace);
			if (stop)co_return 0;
		}

		refGateSession.Say("��������һ�����̳��ˣ�����ѡ�����Ĺ��̳����ٵ�����棬����������Ŀ���");

		//refGateSession.Say("��굥�����ı�����ͷ������׼�˵�λ�����ٵ�����棬����ָ������������");

		{
			auto [stop, wpEntity] = co_await CoEvent<MyEvent::MoveEntity>::Wait(funCancel, [&refSpace](const MyEvent::MoveEntity& ref) {return &ref.wpEntity.lock()->m_refSpace == &refSpace; });
			if (stop)co_return 0;
		}

		refGateSession.Say("�ܺã������Ѿ��������λ�ƶ��ˣ����ڸ���ˢ��һ������󣬲������̳�������󣬹��̳��ͻῪʼ�ɿ�");
		��ԴComponent::Add(refSpace);

		//refGateSession.Say("�ܺã������Ѿ�����ָ���ƶ��ˣ����ڸ���ˢһ���֣��ѱ��ƶ����ָ��������ͻ��Զ����");

		//MonsterComponent::AddMonster(refSpace);

		co_return 0;
	}
}