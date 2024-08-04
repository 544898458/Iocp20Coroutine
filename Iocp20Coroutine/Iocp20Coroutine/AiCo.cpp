#include "StdAfx.h"
#include "AiCo.h"
#include "Entity.h"
#include "../CoRoutine/CoTimer.h"
#include "MySession.h"
#include "MyServer.h"
#include "PlayerSystem.h"
#include "PlayerComponent.h"
#include "../IocpNetwork/StrConv.h"
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
	bool MoveStep(Entity &refThis, const Position localTarget)
	{
		const float step = refThis.m_f移动速度;
		const auto oldPos = refThis.m_Pos;//复制对象，不是引用
		float& x = refThis.m_Pos.x;
		float& z = refThis.m_Pos.z;

		if (std::abs(localTarget.x - x) < step && std::abs(localTarget.z - z) < step) 
		{
			//LOG(INFO) << "已走到" << localTarget.x << "," << localTarget.z << "附近，协程正常退出";
			refThis.Broadcast(MsgChangeSkeleAnim(refThis, "idle"));
			return false;
		}
		
		if (std::abs(localTarget.x - x) >= step)
		{
			x += localTarget.x < x ? -step : step;
		}

		if (std::abs(localTarget.z - z) >= step)
		{
			z += localTarget.z < z ? -step : step;
		}

		refThis.m_eulerAnglesY = CalculateAngle(oldPos, refThis.m_Pos);
		refThis.Broadcast(MsgNotifyPos(refThis));

		return true;
	}
	CoTask<int>WalkToPos(SpEntity spThis, const Position& posTarget, MyServer* pServer, FunCancel& funCancel)
	{
		KeepCancel kc(funCancel);
		const auto localTarget = posTarget;
		auto pLocalServer = pServer;
		spThis->Broadcast(MsgChangeSkeleAnim(*spThis, "run"));

		while (true)
		{
			if (co_await CoTimer::WaitNextUpdate(funCancel))//服务器主工作线程大循环，每次循环触发一次
			{
				LOG(INFO) << "走向" << localTarget << "的协程取消了";
				co_return 0;
			}
			if (spThis->IsDead())
			{
				LOG(INFO) << "自己阵亡，走向" << localTarget << "的协程取消了";
				if (spThis->m_spPlayer)
					spThis->m_spPlayer->m_pSession->Send(MsgSay(StrConv::GbkToUtf8("自己阵亡")));

				co_return 0;
			}

			if (!MoveStep(*spThis, localTarget))
			{
				co_return 0;
			}
		}
		LOG(INFO) << "走向目标协程结束:" << posTarget;
	}

	CoTask<int> WalkToTarget(SpEntity spThis, SpEntity spTarget, MyServer* pServer, FunCancel& funCancel)
	{
		KeepCancel kc(funCancel);
		auto pLocalServer = pServer;
		pLocalServer->m_Sessions.Broadcast(MsgChangeSkeleAnim(*spTarget, "run"));

		while (true)
		{
			if (co_await CoTimer::WaitNextUpdate(funCancel))//服务器主工作线程大循环，每次循环触发一次
			{
				LOG(INFO) << "走向" << spTarget << "的协程取消了";
				co_return 0;
			}
			if (!spThis->DistanceLessEqual(*spTarget, spThis->m_f警戒距离))
			{
				LOG(INFO) << "离开自己的警戒距离" << spTarget << "的协程取消了";
				co_return 0;
			}
			if (spThis->DistanceLessEqual(*spTarget, spThis->m_f攻击距离))
			{
				//LOG(INFO) << "已走到" << spTarget << "附近，协程正常退出";
				pLocalServer->m_Sessions.Broadcast(MsgChangeSkeleAnim(*spTarget, "idle"));
				co_return 0;
			}
			if (!MoveStep(*spThis, spTarget->m_Pos))
			{
				co_return 0;
			}
		}
		LOG(INFO) << "走向目标协程结束:" << spThis->m_Pos;
	}
	CoTask<int> WaitDelete(SpEntity spThis, FunCancel& funCancel)
	{
		KeepCancel kc(funCancel);
		using namespace std;
		if (co_await CoTimer::Wait(3s,funCancel))//服务器主工作线程大循环，每次循环触发一次
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

		while (!co_await CoTimer::Wait(3s, funCancel))
		{
			SpEntity spEntityMonster = std::make_shared<Entity, const Position&, Space&, const std::string& >({ -30.0 }, refSpace, "altman-red");
			spEntityMonster->AddComponentMonster();
			spEntityMonster->m_f警戒距离 = 20;
			spEntityMonster->m_f移动速度 = 0.2f;
			refSpace.setEntity.insert(spEntityMonster);
			spEntityMonster->BroadcastEnter();
		}
		LOG(INFO) << "停止刷怪协程";
		co_return 0;
	}
}