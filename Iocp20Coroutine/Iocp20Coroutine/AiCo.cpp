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

	CoTask<int> Idle(Entity* pEntity, float& x, float& z, std::function<void()>& funCancel)
	{
		KeepCancel kc(funCancel);
		bool stop = false;
		funCancel = [&stop]() {stop = true; };
		while (true)
		{
			if (co_await CoTimer::WaitNextUpdate(funCancel))
			{
				LOG(INFO) << "调用者手动取消了协程Idle";
				co_return 0;
			}
			if (stop)
			{

				LOG(INFO) << "Idle协程正常退出";
				co_return 0;
			}
			//x -= 0.1f;
			if (pEntity->m_spPlayer)
				pEntity->m_spPlayer->m_pSession->m_pServer->m_Sessions.Broadcast(MsgNotifyPos(pEntity));
		}
	}


	CoTask<int> Attack(Entity* pThis, Entity* pDefencer, std::function<void()>& cancel)
	{
		KeepCancel kc(cancel);

		if (pThis->IsDead())
			co_return 0;//自己死亡

		if (pDefencer->IsDead())
			co_return 0;//目标死亡

		pThis->Broadcast(MsgChangeSkeleAnim(pThis, "attack"));//播放攻击动作

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

			if (pThis->IsDead())
				co_return 0;//自己死亡，协程取消

			if (!pThis->DistanceLessEqual(pDefencer, pThis->m_f攻击距离))
				break;//要执行后摇

			if (pDefencer->IsDead())
				break;//要执行后摇

			pDefencer->Hurt(std::get<1>(wait_hurt));//第n次让对方伤1
		}

		if (co_await CoTimer::Wait(3000ms, cancel))//等3秒	后摇
			co_return 0;//协程取消

		if (!pThis->IsDead())
		{
			pThis->Broadcast(MsgChangeSkeleAnim(pThis, "idle"));//播放休闲待机动作
		}

		co_return 0;//协程正常退出
	}
	bool MoveStep(Entity* pThis, const Position localTarget)
	{
		const float step = pThis->m_f移动速度;
		const auto oldPos = pThis->m_Pos;//复制对象，不是引用
		float& x = pThis->m_Pos.x;
		float& z = pThis->m_Pos.z;

		if (std::abs(localTarget.x - x) < step && std::abs(localTarget.z - z) < step) 
		{
			LOG(INFO) << "已走到" << localTarget.x << "," << localTarget.z << "附近，协程正常退出";
			pThis->Broadcast(MsgChangeSkeleAnim(pThis, "idle"));
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

		pThis->m_eulerAnglesY = CalculateAngle(oldPos, pThis->m_Pos);
		pThis->Broadcast(MsgNotifyPos(pThis));

		return true;
	}
	CoTask<int>WalkToPos(Entity* pThis, float& x, float& z, const float targetX, const float targetZ, MyServer* pServer, std::function<void()>& funCancel)
	{
		KeepCancel kc(funCancel);
		const auto localTargetX = targetX;
		const auto localTargetZ = targetZ;
		auto pLocalServer = pServer;
		pThis->Broadcast(MsgChangeSkeleAnim(pThis, "run"));

		while (true)
		{
			if (co_await CoTimer::WaitNextUpdate(funCancel))//服务器主工作线程大循环，每次循环触发一次
			{
				LOG(INFO) << "走向" << localTargetX << "," << localTargetZ << "的协程取消了";
				co_return 0;
			}
			if (pThis->IsDead())
			{
				LOG(INFO) << "自己阵亡，走向" << localTargetX << "," << localTargetZ << "的协程取消了";
				if (pThis->m_spPlayer)
					pThis->m_spPlayer->m_pSession->Send(MsgSay(StrConv::GbkToUtf8("自己阵亡")));

				co_return 0;
			}

			if (!MoveStep(pThis, Position(localTargetX, localTargetZ)))
			{
				co_return 0;
			}
		}
		LOG(INFO) << "走向目标协程结束:" << targetX << "," << targetX;
	}

	CoTask<int> WalkToTarget(Entity* pThis, Entity* pEntity, MyServer* pServer, std::function<void()>& funCancel)
	{
		KeepCancel kc(funCancel);
		auto pLocalServer = pServer;
		pLocalServer->m_Sessions.Broadcast(MsgChangeSkeleAnim(pEntity, "run"));

		while (true)
		{
			if (co_await CoTimer::WaitNextUpdate(funCancel))//服务器主工作线程大循环，每次循环触发一次
			{
				LOG(INFO) << "走向" << pEntity->m_nickName << "的协程取消了";
				co_return 0;
			}
			if (!pThis->DistanceLessEqual(pEntity, pThis->m_f警戒距离))
			{
				LOG(INFO) << "离开自己的警戒距离" << pEntity->m_nickName << "的协程取消了";
				co_return 0;
			}
			if (pThis->DistanceLessEqual(pEntity, pThis->m_f攻击距离))
			{
				LOG(INFO) << "已走到" << pEntity->m_nickName << "附近，协程正常退出";
				pLocalServer->m_Sessions.Broadcast(MsgChangeSkeleAnim(pEntity, "idle"));
				co_return 0;
			}
			if (!MoveStep(pThis, pEntity->m_Pos))
			{
				co_return 0;
			}
		}
		LOG(INFO) << "走向目标协程结束:" << pThis->m_Pos.x << "," << pThis->m_Pos.z;
	}
}