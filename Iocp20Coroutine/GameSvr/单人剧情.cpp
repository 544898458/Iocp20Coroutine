#include "pch.h"
#include "单人剧情.h"
#include "PlayerGateSession_Game.h"
#include "../CoRoutine/CoTimer.h"
#include "../CoRoutine/CoEvent.h"
#include "MyEvent.h"
#include "Entity.h"
#include "EntitySystem.h"
#include "单位组件/资源Component.h"
#include "单位组件/MonsterComponent.h"
#include "单位组件/PlayerComponent.h"
#include "单位组件/AttackComponent.h"
#include "单位组件/造活动单位Component.h"
#include "单位组件/走Component.h"
#include "单位.h"
#include "AiCo.h"

namespace 单人剧情
{

	CoTask<int> Co训练战(Space& refSpace, Entity& ref视口, FunCancel& funCancel, PlayerGateSession_Game& refGateSession)
	{
		KeepCancel kc(funCancel);
		using namespace std;
		PlayerComponent::Say系统(ref视口, "欢迎来到RTS即时战略游戏，现在您要接受基础的训练");

		//auto [stop, msgResponce] = co_await AiCo::ChangeMoney(refGateSession, 100, true, funCancel);
		//if (stop)
		//{
		//	LOG(WARNING) << "ChangeMoney,协程取消";
		//	co_return 0;
		//}
		refSpace.GetSpacePlayer(ref视口).m_u32晶体矿 += 100;

		if (co_await CoTimer::Wait(2s, funCancel))
			co_return 0;

		const 单位类型 类型(单位类型::工程车);
		单位::活动单位配置 配置;
		单位::Find活动单位配置(类型, 配置);
		SpEntity sp工程车 = refSpace.造活动单位(ref视口.m_spPlayer, EntitySystem::GetNickName(ref视口), { 5,10 }, 配置, 类型);

		PlayerComponent::Say系统(ref视口, "请点击“工程车”选中，然后点击“造基地”按钮，再点击空白地面，10秒后就能造出一个基地");

		const auto funSameSpace = [&refSpace, &ref视口](const MyEvent::AddEntity& refAddEntity)
			{ return MyEvent::SameSpace(refAddEntity.wpEntity, refSpace, EntitySystem::GetNickName(ref视口)); };

		Position pos基地;
		{
			const auto& [stop, addEvent] = co_await CoEvent<MyEvent::AddEntity>::Wait(funCancel, funSameSpace);
			if (stop)
				co_return 0;

			pos基地 = addEvent.wpEntity.lock()->Pos();
		}


		if (co_await CoTimer::Wait(10s, funCancel))
			co_return 0;


		PlayerComponent::Say系统(ref视口, "请点击选中基地（圆环特效表示选中），然后点击“造工程车”按钮，5秒后会在基地旁造出一个工程车");

		if (std::get<0>(co_await CoEvent<MyEvent::AddEntity>::Wait(funCancel, funSameSpace)))
			co_return 0;

		PlayerComponent::Say系统(ref视口, "现在您有工程车了，点击选中您的工程车，再点击空旷地面，命令它走向目标点");

		if (std::get<0>(co_await CoEvent<MyEvent::MoveEntity>::Wait(funCancel, [&refSpace, &ref视口](const MyEvent::MoveEntity& ref)
			{
				if (ref.wpEntity.expired())
					return false;

				auto spEnity = ref.wpEntity.lock();
				if (&spEnity->m_refSpace != &refSpace)
					return false;

				if (EntitySystem::GetNickName(*spEnity) != EntitySystem::GetNickName(ref视口))
					return false;

				return true;
			})))
		{
			co_return 0;
		}

		PlayerComponent::Say系统(ref视口, "很好！现在给您刷了一个晶体矿，请点击晶体矿，让工程车在晶体矿和基地之间搬运晶体矿");
		资源Component::Add(refSpace, 晶体矿, { pos基地.x,pos基地.z - 20 });
		资源Component::Add(refSpace, 燃气矿, { pos基地.x + 15,pos基地.z });

		if (std::get<0>(co_await CoEvent<MyEvent::开始采集晶体矿>::Wait(funCancel)))
			co_return 0;

		PlayerComponent::Say系统(ref视口, "很好！您的工程车正在采集晶体矿，请等他把晶体矿运回基地。现在，请选中另一辆工程车去采集燃气矿");
		if (std::get<0>(co_await CoEvent<MyEvent::晶体矿已运回基地>::Wait(funCancel)))
			co_return 0;

		PlayerComponent::Say系统(ref视口, "您的工程车已把第一车晶体矿运到基地，请查看左上角晶体矿数量变化");

		if (co_await CoTimer::Wait(5s, funCancel))
			co_return 0;

		PlayerComponent::Say系统(ref视口, "等您存够20晶体矿后，请选中一辆工程车，然后点击“造民房”按钮，再点击一次空旷地面");
		if (std::get<0>(co_await CoEvent<MyEvent::AddEntity>::Wait(funCancel, funSameSpace)))
			co_return 0;

		PlayerComponent::Say系统(ref视口, "很好，民房可以提升您的活动单位数量上限");

		if (co_await CoTimer::Wait(3s, funCancel))
			co_return 0;

		PlayerComponent::Say系统(ref视口, "等您存够30晶体矿后，请选中一辆工程车，然后点击“造兵厂”按钮，再点击一次空旷地面，就能造出一个兵厂");
		if (std::get<0>(co_await CoEvent<MyEvent::AddEntity>::Wait(funCancel, funSameSpace)))
			co_return 0;

		if (co_await CoTimer::Wait(10s, funCancel))
			co_return 0;

		PlayerComponent::Say系统(ref视口, "请点击选中兵厂（圆环特效表示选中），然后点击“造兵”按钮,10秒后会在兵厂旁造出一个兵");

		if (std::get<0>(co_await CoEvent<MyEvent::AddEntity>::Wait(funCancel, funSameSpace)))
			co_return 0;

		PlayerComponent::Say系统(ref视口, "鼠标点击选中您的兵，再点击地面，可以指挥他走向目标处");

		if (std::get<0>(co_await CoEvent<MyEvent::MoveEntity>::Wait(funCancel, [&refSpace](const MyEvent::MoveEntity& ref) {return &ref.wpEntity.lock()->m_refSpace == &refSpace; })))
			co_return 0;

		PlayerComponent::Say系统(ref视口, "现在已在左边给您刷了一个怪，控制兵走到怪附近，兵会自动打怪。您可以点右下角“取消选中”然后拖动地面看看怪在哪里");
		MonsterComponent::AddMonster(refSpace, 兵, { -30,20 });

		if (std::get<0>(co_await CoEvent<MyEvent::单位阵亡>::Wait(funCancel, [&refSpace](const MyEvent::单位阵亡& ref) {return &ref.wpEntity.lock()->m_refSpace == &refSpace; })))
			co_return 0;

		PlayerComponent::Say系统(ref视口, "您的兵阵亡了。可以多造点兵去围攻敌人");

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

		PlayerComponent::Say系统(ref视口, "恭喜您消灭了敌人！现在左边给您刷了10个敌人。您可以造地堡,让兵进入地堡中，立足防守，再伺机进攻");
		MonsterComponent::AddMonster(refSpace, 兵, { -30.0 }, 10);

		if (std::get<0>(co_await CoEvent<MyEvent::单位阵亡>::Wait(funCancel, [&refSpace](const MyEvent::单位阵亡& ref)
			{
				auto spEntity = ref.wpEntity.lock();
				if (&spEntity->m_refSpace != &refSpace)
					return false;

				return 0 == refSpace.Get怪物单位数();
			})))
		{
			co_return 0;
		}

		PlayerComponent::播放声音(ref视口, "音效/YouWin", "您取得了胜利！您真是指挥天才！");

		PlayerComponent::Send<Msg显示界面>(ref视口.m_spPlayer, { .ui = Msg显示界面::选择地图 });
		co_return 0;
	}
	Position 怪物走向矿附近(const Position& refOld)
	{
		return { -30,30 };
	}

	static void 总教官陈近南说(Entity& ref视口, const std::string& str内容)
	{
		PlayerComponent::剧情对话(ref视口, "图片/总教官", "总教官：陈近南", "", "", "    " + str内容);
		PlayerComponent::播放声音(ref视口, "音效/BUTTON");
	}
	static void 玩家说(Entity& ref视口, PlayerGateSession_Game& refGateSession, const std::string& str内容, const bool b显示退出场景按钮 = false)
	{
		PlayerComponent::剧情对话(ref视口, "", "", "图片/玩家", "玩家：" + refGateSession.NickName(), "    " + str内容, b显示退出场景按钮);
		PlayerComponent::播放声音(ref视口, "音效/BUTTON");
	}

	static CoTask<bool> 等玩家读完(PlayerGateSession_Game& refGateSession, FunCancel& funCancel)
	{
		const auto funSameSession = [&refGateSession](const MyEvent::已阅读剧情对话& ref)
			{ return ref.wpPlayerGateSession.lock().get() == &refGateSession; };
		co_return std::get<0>(co_await CoEvent<MyEvent::已阅读剧情对话>::Wait(funCancel, funSameSession));
	}
	static CoTask<bool> Is战斗结束(Space& refSpace, Entity& ref视口, PlayerGateSession_Game& refGateSession, FunCancel& funCancel)
	{
#define _等玩家读完	{if (co_await 等玩家读完(refGateSession, funCancel))co_return 0;}

		const auto fun总教官陈近南说 = [&ref视口](const std::string& str内容) {总教官陈近南说(ref视口, str内容);};
		const auto fun玩家说 = [&ref视口, &refGateSession](const std::string& str内容) {玩家说(ref视口, refGateSession, str内容);};


		if (0 == refSpace.Get怪物单位数())
		{
			refGateSession.播放声音("音效/YouWin", "您守住了！您真是指挥天才！");

			fun总教官陈近南说(refGateSession.NickName() + "，我果然没有看错你，你是一个指挥天才！");_等玩家读完;
			fun玩家说("承蒙教官谬赞。学生谨遵教诲，自不敢有一丝懈怠，定当不负所望，继续精进。");	_等玩家读完;
			PlayerComponent::剧情对话(ref视口, "图片/总教官", "总教官：陈近南", "", "", "    走你!", true);
			co_return true;
		}
		if (0 == refSpace.Get玩家单位数(refGateSession))
		{
			refGateSession.播放声音("音效/YouLose", "胜败乃兵家常事，请点击右上角“退出场景”离开，然后再次点击“防守战”，就可以重新来过。");
			fun总教官陈近南说("胜败乃兵家常事，我仍然看好你的潜力！");_等玩家读完;
			玩家说(ref视口, refGateSession, "学生此次功败垂成，定会吸取教训，总结经验，再次努力，定不辜负教官的栽培与期望。", true);//	_等玩家读完;
			co_return true;
		}
		co_return false;
	}
	CoTask<int> Co防守战(Space& refSpace, Entity& ref视口, FunCancel& funCancel, PlayerGateSession_Game& refGateSession)
	{
		KeepCancel kc(funCancel);
		using namespace std;

#define _等玩家读完	{if (co_await 等玩家读完(refGateSession, funCancel))co_return 0;}

		const auto fun总教官陈近南说 = [&ref视口](const std::string& str内容) {总教官陈近南说(ref视口, str内容);};
		const auto fun玩家说 = [&ref视口, &refGateSession](const std::string& str内容) {玩家说(ref视口, refGateSession, str内容);};
		fun总教官陈近南说("情报显示：将有大量敌方单位进攻我方基地，请做好准备。");	_等玩家读完;
		fun玩家说("可是我仅受过简单的指挥训练。");							_等玩家读完;
		fun总教官陈近南说("我们会提供足够的初始晶体矿和燃气矿，你不用再从零开始采集。");	_等玩家读完;
		fun玩家说("我还是没有把握。");										_等玩家读完;
		fun总教官陈近南说("我会传授你指挥实战速成口诀，三分钟即可成为实战高手。");	_等玩家读完;
		fun玩家说("真有这种口诀的话就应该在训练战时告诉我啊！");			_等玩家读完;
		fun总教官陈近南说("实战口诀只有在九死一生的实战中才有用！");		_等玩家读完;
		fun玩家说("九死一生？形势严峻啊，我会全力以赴的!");					_等玩家读完;
		fun总教官陈近南说("坦克可以超远距离造成超高伤害，而且是范围伤害，但是坦克锁定炮弹落点后有超长前摇时长，前摇结束后，敌方目标可能已经远离炮弹落点。炮弹落点附近的我方单位也会受到伤害。");_等玩家读完;
		fun玩家说("有点鸡肋的感觉，不是红警中的高速坦克，与星际中的攻城坦克也有些不同。");	_等玩家读完;
		fun总教官陈近南说("…不知道你在说什么…有一种叫“光子炮”的建筑单位，攻击距离很远，仅次于坦克；前摇时长很短，可以快速攻击；缺陷是伤害较低；不过多造点集中防守某一块区域还是很实用的。");_等玩家读完;
		fun玩家说("看来只要有坦克和光子炮就能守住对吗？");					_等玩家读完;
		fun总教官陈近南说("是的，这是其他指挥官总结的经验。大概10个光子炮和5辆坦克沿墙边布置就能守住。当然还有其他多种可行的防守方法，要靠你自行摸索。");_等玩家读完;
		fun玩家说("OK，也不是很复杂。我有信心完成任务！");					_等玩家读完;
		fun总教官陈近南说("走你！");										_等玩家读完;

		PlayerComponent::剧情对话已看完(ref视口);

		PlayerComponent::Say系统(ref视口, "防守战：只要守住，就是胜利！");
		if (co_await CoTimer::Wait(1s, funCancel))
			co_return 0;

		refSpace.GetSpacePlayer(ref视口).m_u32燃气矿 += 300;
		refSpace.GetSpacePlayer(ref视口).m_u32晶体矿 += 1000;
		{
			//refSpace.m_mapPlayer[refGateSession.NickName()].m_u32燃气矿 += 200;
			//auto [stop, msgResponce] = co_await AiCo::ChangeMoney(refGateSession, 1000, true, funCancel);
			//if (stop)
			//{
			//	LOG(WARNING) << "ChangeMoney,协程取消";
			//	co_return 0;
			//}
		}
		//auto [stop, msgResponce] = co_await AiCo::ChangeMoney(refGateSession, 100, true, funCancel);
		//if (stop)
		//{
		//	LOG(WARNING) << "ChangeMoney,协程取消";
		//	co_return 0;
		//}
		{
			{
				const 单位类型 类型(单位类型::工程车);
				单位::活动单位配置 配置;
				单位::Find活动单位配置(类型, 配置);
				refSpace.造活动单位(ref视口.m_spPlayer, EntitySystem::GetNickName(ref视口), { -30, 30 }, 配置, 类型);
			}

			资源Component::Add(refSpace, 晶体矿, { -20, 35 });
			资源Component::Add(refSpace, 晶体矿, { -26, 20 });
			资源Component::Add(refSpace, 燃气矿, { -29, 35 });
			资源Component::Add(refSpace, 燃气矿, { -20, 20 });

		}

		PlayerComponent::Say系统(ref视口, "5秒后将出现第1波怪。您可以造几个地堡，让兵进驻地堡");
		if (co_await CoTimer::Wait(5s, funCancel))
			co_return 0;

		for (int i = 1; i < 20; ++i)
		{
			if (1 < i && co_await Is战斗结束(refSpace, ref视口, refGateSession, funCancel))
				co_return 0;

			refGateSession.Say系统(std::format("第{0}波敌人正向您走来", i));
			auto vecEneity = MonsterComponent::AddMonster(refSpace, i % 2 == 0 ? 兵 : 近战兵, { 48,-48 }, i * 1);
			auto vecEneity2 = MonsterComponent::AddMonster(refSpace, 工蜂, { 47,-47 }, i * 1);
			vecEneity.insert(vecEneity.end(), vecEneity2.begin(), vecEneity2.end());
			for (auto& spEntity : vecEneity)
			{
				spEntity->m_spAttack->m_fun空闲走向此处 = 怪物走向矿附近;
			}

			if (co_await CoTimer::Wait(25s, funCancel))
				co_return 0;
		}

		while (!co_await CoTimer::Wait(5s, funCancel))
		{
			if (co_await Is战斗结束(refSpace, ref视口, refGateSession, funCancel))
				co_return 0;
		}
		co_return 0;
	}
}