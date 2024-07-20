#include "AiCo.h"
#include "Entity.h"
#include "../CoRoutine/CoTimer.h"
#include "MySession.h"
#include "MyServer.h"

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
				LOG(INFO) << "调用者手动取消了协程TraceEnemy";
				co_return 0;
			}
			if (stop)
			{

				LOG(INFO) << "TraceEnemy协程正常退出";
				co_return 0;
			}
			//x -= 0.1f;

			pEntity->m_pSession->m_pServer->m_Sessions.Broadcast(MsgNotifyPos(pEntity, x, z, pEntity->m_hp));
		}
	}


	CoTask<int> Attack(Entity* pEntity, Entity* pDefencer, float& x, float& z, std::function<void()>& cancel)
	{
		using namespace std;
		KeepCancel kc(cancel);

		pEntity->m_pSession->m_pServer->m_Sessions.Broadcast(MsgChangeSkeleAnim(pEntity, "attack"));//播放攻击动作

		if (co_await CoTimer::Wait(3000ms, cancel))//等3秒	前摇
			co_return 0;//协程取消

		pDefencer->Hurt(1);//第一次让对方伤1点生命

		if (co_await CoTimer::Wait(500ms, cancel))//等0.5秒
			co_return 0;//协程取消

		pDefencer->Hurt(3);//第二次让对方伤3点生命

		if (co_await CoTimer::Wait(500ms, cancel))//等0.5秒
			co_return 0;//协程取消

		pDefencer->Hurt(10);//第三次让对方伤10点生命

		if (co_await CoTimer::Wait(3000ms, cancel))//等3秒	后摇
			co_return 0;//协程取消

		pEntity->m_pSession->m_pServer->m_Sessions.Broadcast(MsgChangeSkeleAnim(pEntity, "idle"));//播放休闲待机动作

		if (co_await CoTimer::Wait(5000ms, cancel))//等5秒	公共冷却
			co_return 0;//协程取消

		co_return 0;//协程正常退出
	}

	CoTask<int>WalkToPos(Entity* pEntity, float& x, float& z, const float targetX, const float targetZ, MyServer *pServer, std::function<void()>& funCancel)
	{
		KeepCancel kc(funCancel);
		const auto localTargetX = targetX;
		const auto localTargetZ = targetZ;
		auto pLocalServer = pServer;
		pLocalServer->m_Sessions.Broadcast(MsgChangeSkeleAnim(pEntity, "run"));

		while (true)
		{
			if (co_await CoTimer::WaitNextUpdate(funCancel))//服务器主工作线程大循环，每次循环触发一次
			{
				LOG(INFO) << "走向" << localTargetX << "," << localTargetZ << "的协程取消了";
				co_return 0;
			}

			const auto step = 0.5f;
			if (std::abs(localTargetX - x) < step && std::abs(localTargetZ - z) < step) {
				LOG(INFO) << "已走到" << localTargetX << "," << localTargetZ << "附近，协程正常退出";
				pLocalServer->m_Sessions.Broadcast(MsgChangeSkeleAnim(pEntity, "idle"));
				co_return 0;
			}

			x += localTargetX < x ? -step : step;
			z += localTargetZ < z ? -step : step;

			pLocalServer->m_Sessions.Broadcast(MsgNotifyPos(pEntity, x, z, pEntity->m_hp));
		}
		LOG(INFO) << "走向目标协程结束:" << targetX << "," << targetX;
	}
}