#include "pch.h"
#include "单人剧情.h"
#include "PlayerGateSession_Game.h"
#include "../CoRoutine/CoTimer.h"
#include "../CoRoutine/CoEvent.h"
#include "MyEvent.h"
#include "Entity.h"
#include "资源Component.h"

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
		if (co_await CoTimer::Wait(1s, funCancel))
			co_return 0;

		refGateSession.Say("请单击“造基地”按钮，3秒后就能造出一个基地");


		auto funSameSpace = [&refSpace](const WpEntity& refWpEntity) { return MyEvent::SameSpace(refWpEntity, refSpace); };
		{
			auto [stop, wpEntity] = co_await CoEvent<WpEntity>::Wait(funCancel, funSameSpace);
			if (stop)co_return 0;
		}

		refGateSession.Say("请单击选中基地（暗红色特效表示选中），然后点击“造兵”按钮");

		{
			auto [stop, wpEntity] = co_await CoEvent<WpEntity>::Wait(funCancel, funSameSpace);
			if (stop)co_return 0;
		}

		refGateSession.Say("现在您有一辆工程车了，单击选中您的工程车，再点击地面，命令它走向目标点");

		//refGateSession.Say("鼠标单击您的兵（镜头焦点会对准此单位），再点击地面，可以指挥他走向点击处");

		{
			auto [stop, wpEntity] = co_await CoEvent<MyEvent::MoveEntity>::Wait(funCancel, [&refSpace](const MyEvent::MoveEntity& ref) {return &ref.wpEntity.lock()->m_refSpace == &refSpace; });
			if (stop)co_return 0;
		}

		refGateSession.Say("很好！，您已经可以命令单位移动了，现在给您刷了一个晶体矿，操作工程车走向晶体矿，工程车就会开始采矿");
		资源Component::Add(refSpace);

		//refGateSession.Say("很好！，您已经可以指挥移动了，现在给您刷一个怪，把兵移动到怪附近，兵就会自动打怪");

		//MonsterComponent::AddMonster(refSpace);

		co_return 0;
	}
}