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
#include "单位组件/PlayerComponent.h"
#include "../IocpNetwork/StrConv.h"
#include "PlayerGateSession_Game.h"
#include "MyEvent.h"
#include "SpaceSystem.h"
#include "单位组件/AttackComponent.h"
#include "单位组件/DefenceComponent.h"
#include "单位.h"
#include "单位组件/资源Component.h"
#include "单位组件/临时阻挡Component.h"
#include "单位组件/BuildingComponent.h"
#include "单位组件/走Component.h"

template<class T> void SendToWorldSvr转发(const T& msg, const uint64_t idGateSession);

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

	CoTask<int> 多人联机地图(Space& refSpace, const uint16_t u16半径, FunCancel& funCancel)
	{
		KeepCancel kc(funCancel);
		using namespace std;
		do
		{
			if (std::rand() % 10 == 0)
			{
				//throw std::exception("抛出异常");
			}

			if (refSpace.Get资源单位数(燃气矿) < 10)//如果资源少于10个，就刷
			{
				const Position pos = 在方块里随机一个点(u16半径);
				资源Component::Add(refSpace, 燃气矿, pos);
				资源Component::Add(refSpace, 晶体矿, { pos.x + 10, 0, pos.z });
				资源Component::Add(refSpace, 晶体矿, { pos.x,0,pos.z + 10 });
				资源Component::Add(refSpace, 晶体矿, { pos.x,0,pos.z - 10 });

			}
			if (refSpace.Get怪物单位数() < 6)
			{
				const Rect rect = { {-u16半径, 0.f, -u16半径},{u16半径, 0.f, u16半径} };
				{
					const Position pos = { rect.pos左上.x + std::rand() % rect.宽Int32(), 0, rect.pos左上.z + std::rand() % rect.高Int32() };
					SpaceSystem::AddMonster(refSpace, 枪虫怪, pos);
				}
				{
					const Position pos = { rect.pos左上.x + std::rand() % rect.宽Int32(), 0, rect.pos左上.z + std::rand() % rect.高Int32() };
					SpaceSystem::AddMonster(refSpace, 近战虫怪, pos);
				}

			}

		} while (!co_await CoTimer::Wait(2s, funCancel));

		LOG(INFO) << "停止";
		co_return 0;
	}

	CoTask<std::tuple<bool, MsgChangeMoneyResponce>> ChangeMoney(PlayerGateSession_Game& refSession, int32_t changeMoney, bool addMoney, FunCancel& funCancel)
	{
		if (changeMoney < 0)
		{
			LOG(ERROR) << "";
			_ASSERT(false);
			co_return std::make_tuple(false, MsgChangeMoneyResponce());
		}
		KeepCancel kc(funCancel);
		using namespace std;

		auto tuple = co_await CoRpc<MsgChangeMoneyResponce>::Send<MsgChangeMoney>({ .addMoney = addMoney,.changeMoney = changeMoney },
			[&refSession](const MsgChangeMoney& msg) {SendToWorldSvr转发<MsgChangeMoney>(msg, refSession.m_idPlayerGateSession); }, funCancel);//以同步编程的方式，向另一个服务器发送请求并等待返回
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
				LOG(WARNING) << "协程RPC打断,error=" << responce.error << ",finalMoney=" << responce.finalMoney << ",rpcSn=" << responce.rpcSnId;
				co_return 0;
			}
			//LOG(INFO) << "ChangeMoney返回,error=" << responce.error << ",finalMoney=" << responce.finalMoney;
		}

		co_return 0;
	}
}