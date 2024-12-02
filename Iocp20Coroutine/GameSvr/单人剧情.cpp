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
#include "AttackComponent.h"
#include "造活动单位Component.h"
#include "走Component.h"
#include "单位.h"
#include "AiCo.h"

namespace 单人剧情
{

	CoTask<int> Co训练战(Space& refSpace, FunCancel& funCancel, PlayerGateSession_Game& refGateSession)
	{
		KeepCancel kc(funCancel);
		using namespace std;
		refGateSession.Say系统("欢迎来到即时策略游戏单人剧情");
		if (co_await CoTimer::Wait(2s, funCancel))
			co_return 0;

		const 活动单位类型 类型(活动单位类型::工程车);
		单位::活动单位配置 配置;
		单位::Find活动单位配置(类型, 配置);
		SpEntity sp工程车 = 造活动单位Component::造活动单位(refGateSession, { 30,30 }, 配置, 类型);

		refGateSession.Say系统("请单击“工程车”选中，然后单击“造基地”按钮，再点击空白地面，3秒后就能造出一个基地");

		const auto funSameSpace = [&refSpace, &refGateSession](const MyEvent::AddEntity& refAddEntity) { return MyEvent::SameSpace(refAddEntity.wpEntity, refSpace, refGateSession); };

		Position pos基地;
		{
			const auto& [stop, addEvent] = co_await CoEvent<MyEvent::AddEntity>::Wait(funCancel, funSameSpace);
			if (stop)
				co_return 0;

			pos基地 = addEvent.wpEntity.lock()->m_Pos;
		}

		refGateSession.Say系统("请单击选中基地（暗红色特效表示选中），然后点击“造工程车”按钮,2秒后会在基地旁造出一个工程车");

		if (std::get<0>(co_await CoEvent<MyEvent::AddEntity>::Wait(funCancel, funSameSpace)))
			co_return 0;

		refGateSession.Say系统("现在您有一辆工程车了，单击选中您的工程车，再点击地面，命令它走向目标点");

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
		{
			co_return 0;
		}

		refGateSession.Say系统("很好！现在给您刷了一个晶体矿，请点击晶体矿，让工程车在晶体矿和基地之间搬运晶体矿");
		资源Component::Add(refSpace, 晶体矿, { pos基地.x,pos基地.z - 20 });
		资源Component::Add(refSpace, 燃气矿, { pos基地.x + 30,pos基地.z });

		if (std::get<0>(co_await CoEvent<MyEvent::开始采集晶体矿>::Wait(funCancel)))
			co_return 0;

		refGateSession.Say系统("很好！您的工程车正在采集晶体矿，请等他把晶体矿运回基地");

		if (std::get<0>(co_await CoEvent<MyEvent::晶体矿已运回基地>::Wait(funCancel)))
			co_return 0;

		refGateSession.Say系统("您的工程车已把第一车晶体矿运到基地，请查看左上角晶体矿数量变化");

		if (co_await CoTimer::Wait(3s, funCancel))
			co_return 0;

		refGateSession.Say系统("请单击“造兵厂”按钮");
		if (std::get<0>(co_await CoEvent<MyEvent::AddEntity>::Wait(funCancel, funSameSpace)))
			co_return 0;

		refGateSession.Say系统("请单击选中兵厂（暗红色特效表示选中），然后点击“造兵”按钮,2秒后会在兵厂旁造出一个兵");

		if (std::get<0>(co_await CoEvent<MyEvent::AddEntity>::Wait(funCancel, funSameSpace)))
			co_return 0;

		refGateSession.Say系统("鼠标单击您的兵，再点击地面，可以指挥他走向目标处");

		if (std::get<0>(co_await CoEvent<MyEvent::MoveEntity>::Wait(funCancel, [&refSpace](const MyEvent::MoveEntity& ref) {return &ref.wpEntity.lock()->m_refSpace == &refSpace; })))
			co_return 0;

		refGateSession.Say系统("现在已给您刷了一个怪，控制兵走到怪附近，兵会自动打怪");
		MonsterComponent::AddMonster(refSpace, { -30.0 });

		if (std::get<0>(co_await CoEvent<MyEvent::单位阵亡>::Wait(funCancel, [&refSpace](const MyEvent::单位阵亡& ref) {return &ref.wpEntity.lock()->m_refSpace == &refSpace; })))
			co_return 0;

		refGateSession.Say系统("您的兵阵亡了。可以造多点兵去围攻敌人");

		if (std::get<0>(co_await CoEvent<MyEvent::单位阵亡>::Wait(funCancel, [&refSpace](const MyEvent::单位阵亡& ref)
			{
				auto spEntity = ref.wpEntity.lock();
				if (&spEntity->m_refSpace != &refSpace)
					return false;

				return !spEntity->m_spPlayer;//怪物阵亡
			})))
		{
			co_return 0;
		}

		refGateSession.Say系统("恭喜您消灭了敌人！现在给您刷了10个敌人。您可以造地堡,让兵进入地堡中，立足防守，再伺机进攻");
		MonsterComponent::AddMonster(refSpace, { -30.0 }, 10);

		if (std::get<0>(co_await CoEvent<MyEvent::单位阵亡>::Wait(funCancel, [&refSpace](const MyEvent::单位阵亡& ref)
			{
				auto spEntity = ref.wpEntity.lock();
				if (&spEntity->m_refSpace != &refSpace)
					return false;

				for (const auto [k, v] : refSpace.m_mapEntity)
				{
					if (v->IsDead())
						continue;

					if (nullptr == v->m_spMonster)
						continue;

					if (nullptr == v->m_spPlayer)
						return false;//还有怪活着
				}

				return true;
			})))
		{
			co_return 0;
		}

		refGateSession.Say系统("您取得了胜利！您是指挥天才！");
		refGateSession.Send<Msg显示界面>({ .ui = Msg显示界面::选择地图 });
		co_return 0;
	}
	Position 怪物闲逛(const Position& refOld)
	{
		return { -30,30 };
	}
	CoTask<int> Co防守战(Space& refSpace, FunCancel& funCancel, PlayerGateSession_Game& refGateSession)
	{
		KeepCancel kc(funCancel);
		using namespace std;
		refGateSession.Say系统("防守战：只要守住，就是胜利！");
		if (co_await CoTimer::Wait(1s, funCancel))
			co_return 0;

		//co_await AiCo::ChangeMoney(refGateSession, 1000, true, funCancel);
		{
			const 活动单位类型 类型(活动单位类型::工程车);
			单位::活动单位配置 配置;
			单位::Find活动单位配置(类型, 配置);
			SpEntity sp工程车 = 造活动单位Component::造活动单位(refGateSession, { -30, 30 }, 配置, 类型);

			资源Component::Add(refSpace, 晶体矿, { -20, 30 });
			资源Component::Add(refSpace, 晶体矿, { -25, 25});
			资源Component::Add(refSpace, 燃气矿, { -25, 30 });
			资源Component::Add(refSpace, 燃气矿, { -20, 25 });

		}
		for (int i = 0; i < 15; ++i)
		{
			if (co_await CoTimer::Wait(20s, funCancel))
				co_return 0;

			auto vecEneity = MonsterComponent::AddMonster(refSpace, { 48,-48 }, i);
			for (auto& spEntity : vecEneity)
			{
				//if (spEntity->m_sp走)
					//spEntity->m_sp走->WalkToPos({ -30, 30 });

				spEntity->m_spAttack->m_fun空闲走向此处 = 怪物闲逛;
			}
		}

		//while (true)
		//{
		//	if (co_await CoTimer::Wait(5s, funCancel))
		//		co_return 0;

		//	for (auto [id, spEntity] : refSpace.m_mapEntity)
		//	{
		//		if (spEntity->m_spPlayer)
		//			continue;

		//		if (spEntity->m_sp走)//让空闲的怪走向目标
		//		{
		//			走Component::Cancel所有包含走路的协程(*spEntity); //TryCancel();
		//			spEntity->m_sp走->WalkToPos({ -30, 30 });
		//		}
		//	}
		//}

		co_return 0;
	}
}