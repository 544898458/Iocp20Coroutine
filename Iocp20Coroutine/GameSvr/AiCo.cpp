#include "pch.h"
#include "AiCo.h"
#include "Entity.h"
#include "EntitySystem.h"
#include "../CoRoutine/CoTimer.h"
#include "../CoRoutine/CoRpc.h"
#include "../CoRoutine/CoEvent.h"
#include "GameSvrSession.h"
#include "GameSvr.h"
#include "PlayerSystem.h"
#include "PlayerComponent.h"
#include "../IocpNetwork/StrConv.h"
#include "PlayerGateSession_Game.h"
#include "MyEvent.h"
#include "MonsterComponent.h"
#include "RecastNavigationCrowd.h"
#include "AttackComponent.h"
#include "DefenceComponent.h"
#include "��λ.h"
#include "��ԴComponent.h"

template<class T> void SendToWorldSvr(const T& msg, const uint64_t idGateSession);

namespace AiCo
{
	//CoTask<int> Idle(SpEntity spEntity, FunCancel& funCancel)
	//{
	//	KeepCancel kc(funCancel);
	//	while (true)
	//	{
	//		if (co_await CoTimer::WaitNextUpdate(funCancel))
	//		{
	//			//LOG(INFO) << "�������ֶ�ȡ����Э��Idle";
	//			co_return 0;
	//		}
	//	}
	//}

	/// <summary>
	/// 
	/// </summary>
	/// <param name="refThis"></param>
	/// <param name="localTarget"></param>
	/// <returns>�Ƿ�Ҫ����һ��</returns>
	bool ���ߵ�Ŀ�긽��(Entity& refThis, const Position localTarget, const float f����Ŀ��С�ڴ˾���ͣ�� = 0)
	{
		const float step = std::max(refThis.m_�ٶ�ÿ֡�ƶ�����, f����Ŀ��С�ڴ˾���ͣ��);
		float& x = refThis.m_Pos.x;
		float& z = refThis.m_Pos.z;
		if (std::abs(localTarget.x - x) < step && std::abs(localTarget.z - z) < step)
		{
			//LOG(INFO) << "���ߵ�" << localTarget.x << "," << localTarget.z << "������Э�������˳�";
			EntitySystem::BroadcastChangeSkeleAnimIdle(refThis);
			return true;
		}

		//refThis.m_eulerAnglesY = CalculateAngle(refThis.m_Pos, localTarget);
		refThis.BroadcastNotifyPos();

		return false;
	}
	CoTaskBool WalkToPos(Entity& refThis, const Position posTarget, FunCancel& funCancel, const float f����Ŀ��С�ڴ˾���ͣ��)
	{
		if (!refThis.m_refSpace.CrowdTool��վ��(posTarget))
		{
			LOG(INFO) << posTarget << "����վ��";
			co_return false;
		}
		const auto posOld = refThis.m_Pos;
		RecastNavigationCrowd rnc(refThis, posTarget);
		KeepCancel kc(funCancel);
		const auto posLocalTarget = posTarget;
		refThis.BroadcastChangeSkeleAnim("run");
		CoEvent<MyEvent::MoveEntity>::OnRecvEvent(false, { refThis.weak_from_this() });
		while (true)
		{
			if (co_await CoTimer::WaitNextUpdate(funCancel))//�������������̴߳�ѭ����ÿ��ѭ������һ��
			{
				LOG(INFO) << "����" << posLocalTarget << "��Э��ȡ����";
				co_return true;
			}
			if (refThis.IsDead())
			{
				LOG(INFO) << "�Լ�����������" << posLocalTarget << "��Э��ȡ����";
				PlayerComponent::Say(refThis, "�Լ�����", SayChannel::ϵͳ);

				co_return true;
			}

			if (���ߵ�Ŀ�긽��(refThis, posLocalTarget, f����Ŀ��С�ڴ˾���ͣ��))
			{
				//EntitySystem::BroadcastEntity����(refThis, "���ߵ�Ŀ�긽��");
				co_return false;
			}

			//EntitySystem::BroadcastEntity����(refThis, std::format("��Ŀ��{0}��", (int)refThis.m_Pos.Distance(posTarget)));
		}
		LOG(INFO) << "����Ŀ��Э�̽���:" << posTarget;
		co_return false;
	}

	CoTaskBool WalkToTarget(Entity& refThis, SpEntity spTarget, FunCancel& funCancel, const bool b��龯�����)
	{
		RecastNavigationCrowd rnc(refThis, spTarget->m_Pos);
		KeepCancel kc(funCancel);

		refThis.BroadcastChangeSkeleAnim("run");
		Position posOld;
		while (true)
		{
			if (co_await CoTimer::WaitNextUpdate(funCancel))//�������������̴߳�ѭ����ÿ��ѭ������һ��
			{
				LOG(INFO) << "����" << spTarget << "��Э��ȡ����";
				co_return true;
			}
			if (refThis.IsDead())
			{
				LOG(INFO) << "�Լ�����������[" << spTarget->NickName() << "]��Э��ȡ����";
				PlayerComponent::Say(refThis, "�Լ�����", SayChannel::ϵͳ);

				co_return false;
			}
			if (b��龯����� && !refThis.DistanceLessEqual(*spTarget, refThis.m_f�������))
			{
				LOG(INFO) << "�뿪�Լ��ľ������" << spTarget << "��Э��ȡ����";
				co_return false;
			}
			if (refThis.DistanceLessEqual(*spTarget, refThis.��������()))
			{
				//LOG(INFO) << "���ߵ�" << spTarget << "������Э�������˳�";
				EntitySystem::BroadcastChangeSkeleAnimIdle(refThis);
				//EntitySystem::BroadcastEntity����(refThis, "���ߵ�Ŀ�긽��");
				co_return false;
			}

			if (posOld != spTarget->m_Pos)
			{
				rnc.SetMoveTarget(spTarget->m_Pos);
				posOld = spTarget->m_Pos;
			}

			if (���ߵ�Ŀ�긽��(refThis, spTarget->m_Pos))
			{
				co_return false;
			}
			//EntitySystem::BroadcastEntity����(refThis, std::format("��Ŀ��{0}��", (int)refThis.Distance(*spTarget)));
		}
		LOG(INFO) << "����Ŀ��Э�̽���:" << refThis.m_Pos;
		co_return false;
	}
	CoTask<int> SpawnMonster(Space& refSpace, FunCancel& funCancel)
	{
		KeepCancel kc(funCancel);
		using namespace std;
		do
		{
			//�����Դ����10������ˢ
			if (refSpace.Get��Դ��λ��(ȼ����) < 10)
			{
				const Rect rect = { {-100, -100},{100, 100} };
				const Position pos = { rect.pos����.x + std::rand() % rect.��Int32(), rect.pos����.z + std::rand() % rect.��Int32() };
				��ԴComponent::Add(refSpace, ȼ����, pos);
				��ԴComponent::Add(refSpace, �����, { pos.x,pos.z + 10 });

			}
			if (refSpace.Get���ﵥλ��() < 5)
			{
				const Rect rect = { {-100, -100},{100, 100} };
				const Position pos = { rect.pos����.x + std::rand() % rect.��Int32(), rect.pos����.z + std::rand() % rect.��Int32() };
				MonsterComponent::AddMonster(refSpace, ��, pos);

			}
			//for (int i = 0; i < 100 && !co_await CoTimer::Wait(20s, funCancel); ++i)
			//{
			//	MonsterComponent::AddMonster(refSpace, ��, { -30.0 });
			//}
		} while (!co_await CoTimer::Wait(2s, funCancel));

		LOG(INFO) << "ֹͣ";
		co_return 0;
	}

	CoTask<std::tuple<bool, MsgChangeMoneyResponce>> ChangeMoney(PlayerGateSession_Game& refSession, int32_t changeMoney, bool addMoney, FunCancel& funCancel)
	{
		if (changeMoney < 0)
		{
			assert(false);
			co_return std::make_tuple(false, MsgChangeMoneyResponce());
		}
		KeepCancel kc(funCancel);
		using namespace std;

		auto tuple = co_await CoRpc<MsgChangeMoneyResponce>::Send<MsgChangeMoney>({ .addMoney = addMoney,.changeMoney = changeMoney },
			[&refSession](const MsgChangeMoney& msg) {SendToWorldSvr<MsgChangeMoney>(msg, refSession.m_idPlayerGateSession); }, funCancel);//��ͬ����̵ķ�ʽ������һ���������������󲢵ȴ�����
		const auto& responce = std::get<1>(tuple);
		if (std::get<0>(tuple))
		{
			LOG(WARNING) << "Э��RPC���,error=" << responce.error << ",finalMoney=" << responce.finalMoney;
			co_return tuple;
		}

		//LOG(INFO) << "Э��RPC����,error=" << responce.error << ",finalMoney=" << responce.finalMoney;

		refSession.Send(MsgNotifyMoney{ .finalMoney = responce.finalMoney });

		co_return tuple;
	}

	CoTask<int> AddMoney(PlayerGateSession_Game& refSession, FunCancel& funCancel)
	{
		KeepCancel kc(funCancel);
		using namespace std;

		while (!co_await CoTimer::Wait(100ms, funCancel))
		{
			const auto& [stop, responce] = co_await ChangeMoney(refSession, 2, true, funCancel);
			if (stop)
			{
				LOG(WARNING) << "Э��RPC���,error=" << responce.error << ",finalMoney=" << responce.finalMoney << ",rpcSn=" << responce.msg.rpcSnId;
				co_return 0;
			}
			//LOG(INFO) << "ChangeMoney����,error=" << responce.error << ",finalMoney=" << responce.finalMoney;
		}

		co_return 0;
	}
}