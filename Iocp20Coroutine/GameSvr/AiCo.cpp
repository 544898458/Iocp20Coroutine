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
#include "��λ���/PlayerComponent.h"
#include "../IocpNetwork/StrConv.h"
#include "PlayerGateSession_Game.h"
#include "MyEvent.h"
#include "��λ���/MonsterComponent.h"
#include "RecastNavigationCrowd.h"
#include "��λ���/AttackComponent.h"
#include "��λ���/DefenceComponent.h"
#include "��λ.h"
#include "��λ���/��ԴComponent.h"
#include "��λ���/��ʱ�赲Component.h"
#include "��λ���/BuildingComponent.h"

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
		auto& x = refThis.Pos().x;
		auto& z = refThis.Pos().z;
		if (std::abs(localTarget.x - x) < step && std::abs(localTarget.z - z) < step)
		{
			//LOG(INFO) << "���ߵ�" << localTarget.x << "," << localTarget.z << "������Э�������˳�";
			EntitySystem::BroadcastChangeSkeleAnimIdle(refThis);
			return true;
		}

		//refThis.m_eulerAnglesY = CalculateAngle(refThis.Pos(), localTarget);
		refThis.BroadcastNotifyPos();

		return false;
	}
	CoTaskBool WalkToPos(Entity& refThis, const Position posTarget, FunCancel& funCancel, const float f����Ŀ��С�ڴ˾���ͣ��)
	{
		if (refThis.IsDead())
		{
			LOG(WARNING) << posTarget << ",������������";
			co_return false;
		}
		if (!refThis.m_refSpace.CrowdTool��վ��(posTarget))
		{
			LOG(INFO) << posTarget << "����վ��";
			co_return false;
		}
		const auto posOld = refThis.Pos();
		���λ����·���赲 _(refThis);
		RecastNavigationCrowd rnc(refThis, posTarget);
		KeepCancel kc(funCancel);
		const auto posLocalTarget = posTarget;
		refThis.BroadcastChangeSkeleAnim("run");
		CoEvent<MyEvent::MoveEntity>::OnRecvEvent({ refThis.weak_from_this() });
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
				//PlayerComponent::Say(refThis, "�Լ�����", SayChannel::ϵͳ);

				co_return true;
			}

			if (���ߵ�Ŀ�긽��(refThis, posLocalTarget, f����Ŀ��С�ڴ˾���ͣ��))
			{
				if (refThis.m_spPlayer)
				{
					int a = 0;
				}

				for (int i = 0; i < 30; ++i)
				{
					auto wp��� = refThis.m_refSpace.Get�����Entity(refThis, Space::�ѷ�, [](const Entity&) {return true; });
					if (wp���.expired())
						co_return false;

					Entity& ref��� = *wp���.lock();

					if (!refThis.DistanceLessEqual(ref���, 2.0f))
					{
						//LOG(INFO) << "����û���ѷ���λ��ֹͣ����" << posLocalTarget;
						co_return false;
					}
					if (co_await CoTimer::WaitNextUpdate(funCancel))//�������������̴߳�ѭ����ÿ��ѭ������һ��
					{
						LOG(INFO) << "���ߵ�Ŀ�긽��������" << posLocalTarget << "��Э��ȡ����";
						co_return true;
					}
				}

				LOG(INFO) << "���ߵ�Ŀ�긽�������������ѷ���λ," << posLocalTarget;
				co_return false;
			}

			//EntitySystem::BroadcastEntity����(refThis, std::format("��Ŀ��{0}��", (int)refThis.Pos().Distance(posTarget)));
		}
		LOG(INFO) << "����Ŀ��Э�̽���:" << posTarget;
		co_return false;
	}

	CoTaskBool WalkToTarget(Entity& refThis, SpEntity spTarget, FunCancel& funCancel, const bool b��龯�����)
	{
		if(!refThis.m_sp��)
			co_return false;

		if (refThis.IsDead())
		{
			LOG(WARNING) << refThis.NickName() << ",������������";
			co_return false;
		}

		auto posTarget = spTarget->Pos();
		{
			const auto ok = refThis.m_refSpace.CrowdToolFindNerestPos(posTarget);
			LOG_IF(ERROR, !ok) << "";
			assert(ok);
		}
		���λ����·���赲 _(refThis);
		RecastNavigationCrowd rnc(refThis, posTarget);
		KeepCancel kc(funCancel);
		const float f������߳� = BuildingComponent::������߳�(*spTarget);


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
				//PlayerComponent::Say(refThis, "�Լ�����", SayChannel::ϵͳ);

				co_return false;
			}
			if (b��龯����� && !refThis.DistanceLessEqual(*spTarget, refThis.�������()))
			{
				LOG(INFO) << "�뿪�Լ��ľ������" << spTarget << "��Э��ȡ����";
				co_return false;
			}
			const bool b�����ѷ���λ̫�� = EntitySystem::�����ѷ���λ̫��(refThis);
			if (!b�����ѷ���λ̫�� && refThis.DistanceLessEqual(*spTarget, refThis.��������() + f������߳�))
			{
				//LOG(INFO) << "���ߵ�" << spTarget << "������Э�������˳�";
				EntitySystem::BroadcastChangeSkeleAnimIdle(refThis);
				//EntitySystem::BroadcastEntity����(refThis, "���ߵ�Ŀ�긽��");
				co_return false;
			}

			if (posOld != spTarget->Pos())
			{
				rnc.SetMoveTarget(spTarget->Pos());
				posOld = spTarget->Pos();
			}

			if (���ߵ�Ŀ�긽��(refThis, spTarget->Pos()))
			{
				co_return false;
			}
			//EntitySystem::BroadcastEntity����(refThis, std::format("��Ŀ��{0}��", (int)refThis.Distance(*spTarget)));
		}
		LOG(INFO) << "����Ŀ��Э�̽���:" << refThis.Pos();
		co_return false;
	}
	CoTask<int> ����������ͼ(Space& refSpace, FunCancel& funCancel)
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
				��ԴComponent::Add(refSpace, �����, { pos.x + 10,pos.z });
				��ԴComponent::Add(refSpace, �����, { pos.x,pos.z + 10 });
				��ԴComponent::Add(refSpace, �����, { pos.x,pos.z - 10 });

			}
			if (refSpace.Get���ﵥλ��() < 6)
			{
				const Rect rect = { {-100, -100},{100, 100} };
				{
					const Position pos = { rect.pos����.x + std::rand() % rect.��Int32(), rect.pos����.z + std::rand() % rect.��Int32() };
					MonsterComponent::AddMonster(refSpace, ��, pos);
				}
				{
					const Position pos = { rect.pos����.x + std::rand() % rect.��Int32(), rect.pos����.z + std::rand() % rect.��Int32() }; MonsterComponent::AddMonster(refSpace, ��ս��, pos);
					MonsterComponent::AddMonster(refSpace, ��ս��, pos);
				}

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
			LOG(ERROR) << "";
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