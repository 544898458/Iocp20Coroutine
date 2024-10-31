#include "pch.h"
#include "单人剧情.h"
#include "PlayerGateSession_Game.h"
#include "../CoRoutine/CoTimer.h"
#include "../CoRoutine/CoEvent.h"
#include "MyEvent.h"
#include "Entity.h"
#include "资源Component.h"
#include "MonsterComponent.h"
#include "PlayerComponent.h"

namespace 单人剧情
{

	CoTask<int> Co(Space& refSpace, FunCancel& funCancel, PlayerGateSession_Game& refGateSession)
	{
		KeepCancel kc(funCancel);
		using namespace std;
		//PlayerGateSession_Game* pPlayerGateSession = nullptr;
		//{
		//	auto [stop, p] = co_await CoEvent<PlayerGateSession_Game*>::Wait(funCancel);
		//	if (stop)
		//		co_return 0;

		//	pPlayerGateSession = p;
		//}

		//CHECK_NOTNULL_CO_RET_0(pPlayerGateSession);
		refGateSession.Say("欢迎来到即时策略游戏单人剧情");
		if (co_await CoTimer::Wait(2s, funCancel))
			co_return 0;

		refGateSession.Say("请单击“造基地”按钮，3秒后就能造出一个基地");

		const auto funSameSpace = [&refSpace, &refGateSession](const MyEvent::AddEntity& refAddEntity) { return MyEvent::SameSpace(refAddEntity.wpEntity, refSpace, refGateSession); };

		if (std::get<0>(co_await CoEvent<MyEvent::AddEntity>::Wait(funCancel, funSameSpace)))
			co_return 0;

		refGateSession.Say("请单击选中基地（暗红色特效表示选中），然后点击“造兵”按钮,2秒后会在基地旁造出一个工程车");

		if (std::get<0>(co_await CoEvent<MyEvent::AddEntity>::Wait(funCancel, funSameSpace)))
			co_return 0;

		refGateSession.Say("现在您有一辆工程车了，单击选中您的工程车，再点击地面，命令它走向目标点");

		if (std::get<0>(co_await CoEvent<MyEvent::MoveEntity>::Wait(funCancel, [&refSpace, &refGateSession](const MyEvent::MoveEntity& ref)
			{
				if (ref.wpEntity.expired())
					return false;

				auto spEnity = ref.wpEntity.lock();
				if (&spEnity->m_refSpace != &refSpace)
					return false;

				if (!spEnity->m_spPlayer)
					return false;

				if (&spEnity->m_spPlayer->m_refSession != &refGateSession)
					return false;

				return true;
			})))
			co_return 0;

			refGateSession.Say("很好！现在给您刷了一个晶体矿，请点击晶体矿，让工程车在晶体矿和基地之间搬运晶体矿");
			资源Component::Add(refSpace, 晶体矿, { 20,0 });
			资源Component::Add(refSpace, 燃气矿, { 20,30 });

			if (std::get<0>(co_await CoEvent<MyEvent::开始采集晶体矿>::Wait(funCancel)))
				co_return 0;

			refGateSession.Say("很好！您的工程车正在采集晶体矿，请等他把晶体矿运回基地");

			if (std::get<0>(co_await CoEvent<MyEvent::晶体矿已运回基地>::Wait(funCancel)))
				co_return 0;

			refGateSession.Say("您的工程车已把第一车晶体矿运到基地，请查看左上角晶体矿数量变化");

			if (co_await CoTimer::Wait(3s, funCancel))
				co_return 0;

			refGateSession.Say("请单击“造兵厂”按钮");
			if (std::get<0>(co_await CoEvent<MyEvent::AddEntity>::Wait(funCancel, funSameSpace)))
				co_return 0;

			refGateSession.Say("请单击选中兵厂（暗红色特效表示选中），然后点击“造兵”按钮,2秒后会在兵厂旁造出一个兵");

			if (std::get<0>(co_await CoEvent<MyEvent::AddEntity>::Wait(funCancel, funSameSpace)))
				co_return 0;

			refGateSession.Say("鼠标单击您的兵，再点击地面，可以指挥他走向目标处");

			if (std::get<0>(co_await CoEvent<MyEvent::MoveEntity>::Wait(funCancel, [&refSpace](const MyEvent::MoveEntity& ref) {return &ref.wpEntity.lock()->m_refSpace == &refSpace; })))
				co_return 0;

			refGateSession.Say("现在已给您刷了一个怪，控制兵走到怪附近，兵会自动打怪");
			MonsterComponent::AddMonster(refSpace);

			co_return 0;
	}
}