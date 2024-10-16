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
				//LOG(INFO) << "调用者手动取消了协程Idle";
				co_return 0;
			}
		}
	}


	CoTask<int> Attack(SpEntity spThis, SpEntity spDefencer, FunCancel& cancel)
	{
		KeepCancel kc(cancel);

		if (spThis->IsDead())
			co_return 0;//自己死亡

		if (spDefencer->IsDead())
			co_return 0;//目标死亡

		spThis->Broadcast(MsgChangeSkeleAnim(*spThis, "attack"));//播放攻击动作

		using namespace std;


		const std::tuple<std::chrono::milliseconds, int> arrWaitHurt[] =
		{	//三段伤害{每段前摇时长，伤害值}
			{3000ms,1},
			{500ms,3},
			{500ms,10}
		};

		for (auto wait_hurt : arrWaitHurt)
		{
			if (co_await CoTimer::Wait(std::get<0>(wait_hurt), cancel))//等x秒	前摇
				co_return 0;//协程取消

			if (spThis->IsDead())
				co_return 0;//自己死亡，协程取消

			if (!spThis->DistanceLessEqual(*spDefencer, spThis->m_f攻击距离))
				break;//要执行后摇

			if (spDefencer->IsDead())
				break;//要执行后摇

			spDefencer->Hurt(std::get<1>(wait_hurt));//第n次让对方伤1
		}

		if (co_await CoTimer::Wait(3000ms, cancel))//等3秒	后摇
			co_return 0;//协程取消

		if (!spThis->IsDead())
		{
			spThis->Broadcast(MsgChangeSkeleAnim(*spThis, "idle"));//播放休闲待机动作
		}

		co_return 0;//协程正常退出
	}
	/// <summary>
	/// 
	/// </summary>
	/// <param name="refThis"></param>
	/// <param name="localTarget"></param>
	/// <returns>是否还要走下一步</returns>
	bool MoveStep(Entity& refThis, const Position localTarget, RecastNavigationCrowd& rnc)
	{
		const float step = refThis.m_f移动速度;
		const auto oldPos = refThis.m_Pos;//复制对象，不是引用
		float& x = refThis.m_Pos.x;
		float& z = refThis.m_Pos.z;
		rnc.SetMoveTarget(localTarget);
		if (std::abs(localTarget.x - x) < step && std::abs(localTarget.z - z) < step)
		{
			//LOG(INFO) << "已走到" << localTarget.x << "," << localTarget.z << "附近，协程正常退出";
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
			if (co_await CoTimer::WaitNextUpdate(funCancel))//服务器主工作线程大循环，每次循环触发一次
			{
				LOG(INFO) << "走向" << posLocalTarget << "的协程取消了";
				co_return true;
			}
			if (spThis->IsDead())
			{
				LOG(INFO) << "自己阵亡，走向" << posLocalTarget << "的协程取消了";
				if (spThis->m_spPlayer)
					spThis->m_spPlayer->Say("自己阵亡");

				co_return true;
			}

			if (!MoveStep(*spThis, posLocalTarget, rnc))
			{
				co_return false;
			}
		}
		LOG(INFO) << "走向目标协程结束:" << posTarget;
		co_return false;
	}

	CoTaskBool WalkToTarget(SpEntity spThis, SpEntity spTarget, FunCancel& funCancel, const bool b检查警戒距离)
	{
		RecastNavigationCrowd rnc(*spThis, spTarget->m_Pos);
		KeepCancel kc(funCancel);

		spThis->Broadcast(MsgChangeSkeleAnim(*spTarget, "run"));

		while (true)
		{
			if (co_await CoTimer::WaitNextUpdate(funCancel))//服务器主工作线程大循环，每次循环触发一次
			{
				LOG(INFO) << "走向" << spTarget << "的协程取消了";
				co_return true;
			}
			if (spThis->IsDead())
			{
				LOG(INFO) << "自己阵亡，走向[" << spTarget->NickName() << "]的协程取消了";
				if (spThis->m_spPlayer)
					spThis->m_spPlayer->Say("自己阵亡");

				co_return true;
			}
			if (b检查警戒距离 && !spThis->DistanceLessEqual(*spTarget, spThis->m_f警戒距离))
			{
				LOG(INFO) << "离开自己的警戒距离" << spTarget << "的协程取消了";
				co_return true;
			}
			if (spThis->DistanceLessEqual(*spTarget, spThis->m_f攻击距离))
			{
				//LOG(INFO) << "已走到" << spTarget << "附近，协程正常退出";
				spThis->Broadcast(MsgChangeSkeleAnim(*spTarget, "idle"));
				co_return false;
			}
			if (!MoveStep(*spThis, spTarget->m_Pos, rnc))
			{
				co_return true;
			}
		}
		LOG(INFO) << "走向目标协程结束:" << spThis->m_Pos;
		co_return false;
	}
	CoTask<int> WaitDelete(SpEntity spThis, FunCancel& funCancel)
	{
		KeepCancel kc(funCancel);
		using namespace std;
		if (co_await CoTimer::Wait(3s, funCancel))//服务器主工作线程大循环，每次循环触发一次
		{
			LOG(INFO) << "WaitDelete协程取消了";
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
			//spEntityMonster->m_f警戒距离 = 20;
			//spEntityMonster->m_f移动速度 = 0.2f;
			//refSpace.m_mapEntity.insert({ (int64_t)spEntityMonster.get() ,spEntityMonster });
			////LOG(INFO) << "SpawnMonster:" << refSpace.m_mapEntity.size();
			//spEntityMonster->BroadcastEnter();
			MonsterComponent::AddMonster(refSpace);
			co_return 0;
		}
		LOG(INFO) << "停止刷怪协程";
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
			[&refSession](const MsgChangeMoney& msg) {SendToWorldSvr<MsgChangeMoney>(msg, refSession.m_idPlayerGateSession); }, funCancel);//以同步编程的方式，向另一个服务器发送请求并等待返回
		const auto& responce = std::get<1>(tuple);
		if (std::get<0>(tuple))
		{
			LOG(WARNING) << "协程RPC打断,error=" << responce.error << ",finalMoney=" << responce.finalMoney;
			co_return tuple;
		}

		//LOG(INFO) << "协程RPC返回,error=" << responce.error << ",finalMoney=" << responce.finalMoney;

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
				LOG(WARNING) << "协程RPC打断,error=" << responce.error << ",finalMoney=" << responce.finalMoney << ",rpcSn=" << responce.msg.rpcSnId;
				co_return 0;
			}
			//LOG(INFO) << "ChangeMoney返回,error=" << responce.error << ",finalMoney=" << responce.finalMoney;
		}

		co_return 0;
	}
}