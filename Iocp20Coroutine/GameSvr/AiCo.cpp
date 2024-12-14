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
#include "单位.h"
#include "资源Component.h"

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
	//			//LOG(INFO) << "调用者手动取消了协程Idle";
	//			co_return 0;
	//		}
	//	}
	//}

	/// <summary>
	/// 
	/// </summary>
	/// <param name="refThis"></param>
	/// <param name="localTarget"></param>
	/// <returns>是否还要走下一步</returns>
	bool 已走到目标附近(Entity& refThis, const Position localTarget, const float f距离目标小于此距离停下 = 0)
	{
		const float step = std::max(refThis.m_速度每帧移动距离, f距离目标小于此距离停下);
		float& x = refThis.m_Pos.x;
		float& z = refThis.m_Pos.z;
		if (std::abs(localTarget.x - x) < step && std::abs(localTarget.z - z) < step)
		{
			//LOG(INFO) << "已走到" << localTarget.x << "," << localTarget.z << "附近，协程正常退出";
			EntitySystem::BroadcastChangeSkeleAnimIdle(refThis);
			return true;
		}

		//refThis.m_eulerAnglesY = CalculateAngle(refThis.m_Pos, localTarget);
		refThis.BroadcastNotifyPos();

		return false;
	}
	CoTaskBool WalkToPos(Entity& refThis, const Position posTarget, FunCancel& funCancel, const float f距离目标小于此距离停下)
	{
		if (!refThis.m_refSpace.CrowdTool可站立(posTarget))
		{
			LOG(INFO) << posTarget << "不可站立";
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
			if (co_await CoTimer::WaitNextUpdate(funCancel))//服务器主工作线程大循环，每次循环触发一次
			{
				LOG(INFO) << "走向" << posLocalTarget << "的协程取消了";
				co_return true;
			}
			if (refThis.IsDead())
			{
				LOG(INFO) << "自己阵亡，走向" << posLocalTarget << "的协程取消了";
				PlayerComponent::Say(refThis, "自己阵亡", SayChannel::系统);

				co_return true;
			}

			if (已走到目标附近(refThis, posLocalTarget, f距离目标小于此距离停下))
			{
				//EntitySystem::BroadcastEntity描述(refThis, "已走到目标附近");
				co_return false;
			}

			//EntitySystem::BroadcastEntity描述(refThis, std::format("距目标{0}米", (int)refThis.m_Pos.Distance(posTarget)));
		}
		LOG(INFO) << "走向目标协程结束:" << posTarget;
		co_return false;
	}

	CoTaskBool WalkToTarget(Entity& refThis, SpEntity spTarget, FunCancel& funCancel, const bool b检查警戒距离)
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
				PlayerComponent::Say(refThis, "自己阵亡", SayChannel::系统);

				co_return false;
			}
			if (b检查警戒距离 && !refThis.DistanceLessEqual(*spTarget, refThis.m_f警戒距离))
			{
				LOG(INFO) << "离开自己的警戒距离" << spTarget << "的协程取消了";
				co_return false;
			}
			if (refThis.DistanceLessEqual(*spTarget, refThis.攻击距离()))
			{
				//LOG(INFO) << "已走到" << spTarget << "附近，协程正常退出";
				EntitySystem::BroadcastChangeSkeleAnimIdle(refThis);
				//EntitySystem::BroadcastEntity描述(refThis, "已走到目标附近");
				co_return false;
			}

			if (posOld != spTarget->m_Pos)
			{
				rnc.SetMoveTarget(spTarget->m_Pos);
				posOld = spTarget->m_Pos;
			}

			if (已走到目标附近(refThis, spTarget->m_Pos))
			{
				co_return false;
			}
			//EntitySystem::BroadcastEntity描述(refThis, std::format("距目标{0}米", (int)refThis.Distance(*spTarget)));
		}
		LOG(INFO) << "走向目标协程结束:" << refThis.m_Pos;
		co_return false;
	}
	CoTask<int> SpawnMonster(Space& refSpace, FunCancel& funCancel)
	{
		KeepCancel kc(funCancel);
		using namespace std;
		do
		{
			//如果资源少于10个，就刷
			if (refSpace.Get资源单位数(燃气矿) < 10)
			{
				const Rect rect = { {-100, -100},{100, 100} };
				const Position pos = { rect.pos左上.x + std::rand() % rect.宽Int32(), rect.pos左上.z + std::rand() % rect.高Int32() };
				资源Component::Add(refSpace, 燃气矿, pos);
				资源Component::Add(refSpace, 晶体矿, { pos.x,pos.z + 10 });

			}
			if (refSpace.Get怪物单位数() < 5)
			{
				const Rect rect = { {-100, -100},{100, 100} };
				const Position pos = { rect.pos左上.x + std::rand() % rect.宽Int32(), rect.pos左上.z + std::rand() % rect.高Int32() };
				MonsterComponent::AddMonster(refSpace, 兵, pos);

			}
			//for (int i = 0; i < 100 && !co_await CoTimer::Wait(20s, funCancel); ++i)
			//{
			//	MonsterComponent::AddMonster(refSpace, 兵, { -30.0 });
			//}
		} while (!co_await CoTimer::Wait(2s, funCancel));

		LOG(INFO) << "停止";
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