#include "pch.h"
#include "AiCo.h"
#include "Entity.h"
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

template<class T> void SendToWorldSvr(const T& msg, const uint64_t idGateSession);

namespace AiCo
{
	CoTask<int> Idle(SpEntity spEntity, FunCancel& funCancel)
	{
		KeepCancel kc(funCancel);
		while (true)
		{
			if (co_await CoTimer::WaitNextUpdate(funCancel))
			{
				//LOG(INFO) << "�������ֶ�ȡ����Э��Idle";
				co_return 0;
			}
		}
	}


	CoTask<int> Attack(SpEntity spThis, SpEntity spDefencer, FunCancel& cancel)
	{
		KeepCancel kc(cancel);

		if (spThis->IsDead())
			co_return 0;//�Լ�����

		if (spDefencer->IsDead())
			co_return 0;//Ŀ������

		spThis->Broadcast(MsgChangeSkeleAnim(*spThis, "attack"));//���Ź�������

		using namespace std;


		const std::tuple<std::chrono::milliseconds, int> arrWaitHurt[] =
		{	//�����˺�{ÿ��ǰҡʱ�����˺�ֵ}
			{3000ms,1},
			{500ms,3},
			{500ms,10}
		};

		for (auto wait_hurt : arrWaitHurt)
		{
			if (co_await CoTimer::Wait(std::get<0>(wait_hurt), cancel))//��x��	ǰҡ
				co_return 0;//Э��ȡ��

			if (spThis->IsDead())
				co_return 0;//�Լ�������Э��ȡ��

			if (!spThis->DistanceLessEqual(*spDefencer, spThis->m_f��������))
				break;//Ҫִ�к�ҡ

			if (spDefencer->IsDead())
				break;//Ҫִ�к�ҡ

			spDefencer->Hurt(std::get<1>(wait_hurt));//��n���öԷ���1
		}

		if (co_await CoTimer::Wait(3000ms, cancel))//��3��	��ҡ
			co_return 0;//Э��ȡ��

		if (!spThis->IsDead())
		{
			spThis->Broadcast(MsgChangeSkeleAnim(*spThis, "idle"));//�������д�������
		}

		co_return 0;//Э�������˳�
	}
	/// <summary>
	/// 
	/// </summary>
	/// <param name="refThis"></param>
	/// <param name="localTarget"></param>
	/// <returns>�Ƿ�Ҫ����һ��</returns>
	bool MoveStep(Entity& refThis, const Position localTarget, RecastNavigationCrowd& rnc)
	{
		const float step = refThis.m_f�ƶ��ٶ�;
		const auto oldPos = refThis.m_Pos;//���ƶ��󣬲�������
		float& x = refThis.m_Pos.x;
		float& z = refThis.m_Pos.z;
		rnc.SetMoveTarget(localTarget);
		if (std::abs(localTarget.x - x) < step && std::abs(localTarget.z - z) < step)
		{
			//LOG(INFO) << "���ߵ�" << localTarget.x << "," << localTarget.z << "������Э�������˳�";
			refThis.Broadcast(MsgChangeSkeleAnim(refThis, "idle"));
			return false;
		}

		//if (std::abs(localTarget.x - x) >= step)
		//{
		//	x += localTarget.x < x ? -step : step;
		//}

		//if (std::abs(localTarget.z - z) >= step)
		//{
		//	z += localTarget.z < z ? -step : step;
		//}

		refThis.m_eulerAnglesY = CalculateAngle(oldPos, refThis.m_Pos);
		refThis.Broadcast(MsgNotifyPos(refThis));

		return true;
	}
	CoTaskBool WalkToPos(SpEntity spThis, const Position& posTarget, FunCancel& funCancel)
	{
		const auto posOld = spThis->m_Pos;
		RecastNavigationCrowd rnc(*spThis, posTarget);
		KeepCancel kc(funCancel);
		const auto posLocalTarget = posTarget;
		spThis->Broadcast(MsgChangeSkeleAnim(*spThis, "run"));
		CoEvent<MyEvent::MoveEntity>::OnRecvEvent(false, { spThis->weak_from_this() });
		while (true)
		{
			if (co_await CoTimer::WaitNextUpdate(funCancel))//�������������̴߳�ѭ����ÿ��ѭ������һ��
			{
				LOG(INFO) << "����" << posLocalTarget << "��Э��ȡ����";
				co_return true;
			}
			if (spThis->IsDead())
			{
				LOG(INFO) << "�Լ�����������" << posLocalTarget << "��Э��ȡ����";
				if (spThis->m_spPlayer)
					spThis->m_spPlayer->Say("�Լ�����");

				co_return true;
			}

			if (!MoveStep(*spThis, posLocalTarget, rnc))
			{
				co_return false;
			}
		}
		LOG(INFO) << "����Ŀ��Э�̽���:" << posTarget;
		co_return false;
	}

	CoTaskBool WalkToTarget(SpEntity spThis, SpEntity spTarget, FunCancel& funCancel, const bool b��龯�����)
	{
		RecastNavigationCrowd rnc(*spThis, spTarget->m_Pos);
		KeepCancel kc(funCancel);

		spThis->Broadcast(MsgChangeSkeleAnim(*spTarget, "run"));

		while (true)
		{
			if (co_await CoTimer::WaitNextUpdate(funCancel))//�������������̴߳�ѭ����ÿ��ѭ������һ��
			{
				LOG(INFO) << "����" << spTarget << "��Э��ȡ����";
				co_return true;
			}
			if (spThis->IsDead())
			{
				LOG(INFO) << "�Լ�����������[" << spTarget->NickName() << "]��Э��ȡ����";
				if (spThis->m_spPlayer)
					spThis->m_spPlayer->Say("�Լ�����");

				co_return true;
			}
			if (b��龯����� && !spThis->DistanceLessEqual(*spTarget, spThis->m_f�������))
			{
				LOG(INFO) << "�뿪�Լ��ľ������" << spTarget << "��Э��ȡ����";
				co_return true;
			}
			if (spThis->DistanceLessEqual(*spTarget, spThis->m_f��������))
			{
				//LOG(INFO) << "���ߵ�" << spTarget << "������Э�������˳�";
				spThis->Broadcast(MsgChangeSkeleAnim(*spTarget, "idle"));
				co_return false;
			}
			if (!MoveStep(*spThis, spTarget->m_Pos, rnc))
			{
				co_return true;
			}
		}
		LOG(INFO) << "����Ŀ��Э�̽���:" << spThis->m_Pos;
		co_return false;
	}
	CoTask<int> WaitDelete(SpEntity spThis, FunCancel& funCancel)
	{
		KeepCancel kc(funCancel);
		using namespace std;
		if (co_await CoTimer::Wait(3s, funCancel))//�������������̴߳�ѭ����ÿ��ѭ������һ��
		{
			LOG(INFO) << "WaitDeleteЭ��ȡ����";
		}
		spThis->m_bNeedDelete = true;
		co_return 0;
	}
	CoTask<int> SpawnMonster(Space& refSpace, FunCancel& funCancel)
	{
		KeepCancel kc(funCancel);
		using namespace std;

		while (!co_await CoTimer::Wait(1000ms, funCancel))
		{
			//SpEntity spEntityMonster = std::make_shared<Entity, const Position&, Space&, const std::string& >({ -30.0 }, refSpace, "altman-red");
			//spEntityMonster->AddComponentAttack();
			//spEntityMonster->AddComponentMonster();
			//spEntityMonster->m_f������� = 20;
			//spEntityMonster->m_f�ƶ��ٶ� = 0.2f;
			//refSpace.m_mapEntity.insert({ (int64_t)spEntityMonster.get() ,spEntityMonster });
			////LOG(INFO) << "SpawnMonster:" << refSpace.m_mapEntity.size();
			//spEntityMonster->BroadcastEnter();
			MonsterComponent::AddMonster(refSpace);
			co_return 0;
		}
		LOG(INFO) << "ֹͣˢ��Э��";
		co_return 0;
	}
	CoTask<std::tuple<bool, MsgChangeMoneyResponce>> ChangeMoney(PlayerGateSession_Game& refSession, uint32_t changeMoney, bool addMoney, FunCancel& funCancel)
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