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
#include "AttackComponent.h"
#include "DefenceComponent.h"

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
		
	/// <summary>
	/// 
	/// </summary>
	/// <param name="refThis"></param>
	/// <param name="localTarget"></param>
	/// <returns>是否还要走下一步</returns>
	bool MoveStep(Entity& refThis, const Position localTarget)
	{
		const float step = refThis.m_速度每帧移动距离;
		float& x = refThis.m_Pos.x;
		float& z = refThis.m_Pos.z;
		if (std::abs(localTarget.x - x) < step && std::abs(localTarget.z - z) < step)
		{
			//LOG(INFO) << "已走到" << localTarget.x << "," << localTarget.z << "附近，协程正常退出";
			refThis.BroadcastChangeSkeleAnim("idle");
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

		refThis.m_eulerAnglesY = CalculateAngle(refThis.m_Pos, localTarget);
		refThis.BroadcastNotifyPos();

		return true;
	}
	CoTaskBool WalkToPos(SpEntity spThis, const Position& posTarget, FunCancel& funCancel)
	{
		const auto posOld = spThis->m_Pos;
		RecastNavigationCrowd rnc(*spThis, posTarget);
		KeepCancel kc(funCancel);
		const auto posLocalTarget = posTarget;
		spThis->BroadcastChangeSkeleAnim("run");
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

			if (!MoveStep(*spThis, posLocalTarget))
			{
				co_return false;
			}
		}
		LOG(INFO) << "走向目标协程结束:" << posTarget;
		co_return false;
	}

	CoTaskBool WalkToTarget(Entity &refThis, SpEntity spTarget, FunCancel& funCancel, const bool b检查警戒距离)
	{
		RecastNavigationCrowd rnc(refThis, spTarget->m_Pos);
		KeepCancel kc(funCancel);

		refThis.BroadcastChangeSkeleAnim("run");
		Position posOld;
		while (true)
		{
			if (co_await CoTimer::WaitNextUpdate(funCancel))//服务器主工作线程大循环，每次循环触发一次
			{
				LOG(INFO) << "走向" << spTarget << "的协程取消了";
				co_return true;
			}
			if (refThis.IsDead())
			{
				LOG(INFO) << "自己阵亡，走向[" << spTarget->NickName() << "]的协程取消了";
				if (refThis.m_spPlayer)
					refThis.m_spPlayer->Say("自己阵亡");

				co_return false;
			}
			if (b检查警戒距离 && !refThis.DistanceLessEqual(*spTarget, refThis.m_f警戒距离))
			{
				LOG(INFO) << "离开自己的警戒距离" << spTarget << "的协程取消了";
				co_return false;
			}
			if (refThis.DistanceLessEqual(*spTarget, refThis.m_f攻击距离))
			{
				//LOG(INFO) << "已走到" << spTarget << "附近，协程正常退出";
				refThis.BroadcastChangeSkeleAnim("idle");
				co_return false;
			}

			if (posOld != spTarget->m_Pos)
			{
				rnc.SetMoveTarget(spTarget->m_Pos);
				posOld = spTarget->m_Pos;
			}

			if (!MoveStep(refThis, spTarget->m_Pos))
			{
				co_return false;
			}
		}
		LOG(INFO) << "走向目标协程结束:" << refThis.m_Pos;
		co_return false;
	}
	CoTask<int> SpawnMonster(Space& refSpace, FunCancel& funCancel)
	{
		KeepCancel kc(funCancel);
		using namespace std;

		while (!co_await CoTimer::Wait(5000ms, funCancel))
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
		}
		LOG(INFO) << "停止刷怪协程";
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