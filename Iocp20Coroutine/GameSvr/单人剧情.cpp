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
#include "单位组件/造建筑Component.h"
#include "单位组件/BuildingComponent.h"
#include "单位组件/找目标走过去Component.h"
#include "单位.h"
#include "AiCo.h"


#define _等玩家读完returnTrue	{if (co_await 等玩家读完(strPlayerNickName, funCancel))co_return true;}
std::weak_ptr<PlayerGateSession_Game> GetPlayerGateSession(const std::string& refStrNickName);
namespace 单人剧情
{
	static void 总教官凌云说(Space& refSpace, const std::string& refStrNickName, const std::string& str内容)
	{
		PlayerComponent::剧情对话(refSpace, refStrNickName, "图片/女教官白色海军服", "总教官：凌云", "", "", "    " + str内容);
		PlayerComponent::播放声音(refStrNickName, "音效/BUTTON", "");
	}
	static void 科学家玛丽亚说(Space& refSpace, const std::string& refStrNickName, const std::string& str内容)
	{
		PlayerComponent::剧情对话(refSpace, refStrNickName, "图片/红发年轻女人", "科学家：玛丽亚", "", "", "    " + str内容);
		PlayerComponent::播放声音(refStrNickName, "音效/BUTTON", "");
	}
	static void 坦克手齐诺维说(Space& refSpace, const std::string& refStrNickName, const std::string& str内容)
	{
		PlayerComponent::剧情对话(refSpace, refStrNickName, "图片/青壮年欧洲男人脸朝右", "坦克手：齐诺维", "", "", "    " + str内容);
		PlayerComponent::播放声音(refStrNickName, "音效/BUTTON", "");
	}
	static void 绿色坦克洪隆说(Space& refSpace, const std::string& refStrNickName, const std::string& str内容)
	{
		PlayerComponent::剧情对话(refSpace, refStrNickName, "图片/绿色坦克头像", "绿色坦克：洪隆", "", "", "    " + str内容);
		PlayerComponent::播放声音(refStrNickName, "音效/BUTTON", "");
	}
	static void 装甲指挥官海因茨说(Space& refSpace, const std::string& refStrNickName, const std::string& str内容)
	{
		PlayerComponent::剧情对话(refSpace, refStrNickName, "图片/大白胡子抽烟", "装甲指挥官：海因茨", "", "", "    " + str内容);
		PlayerComponent::播放声音(refStrNickName, "音效/BUTTON", "");
	}
	static void 房虫胡噜说(Space& refSpace, const std::string& refStrNickName, const std::string& str内容)
	{
		PlayerComponent::剧情对话(refSpace, refStrNickName, "图片/房虫头像", "房虫：胡噜", "", "", "    " + str内容);
		PlayerComponent::播放声音(refStrNickName, "音效/BUTTON", "");
	}
	static void 玩家说(Space& refSpace, const std::string& strPlayerNickName, const std::string& str内容, const bool b显示退出场景按钮 = false)
	{
		PlayerComponent::剧情对话(refSpace, strPlayerNickName, "", "", "图片/指挥学员学员青灰色军服", "玩家：" + strPlayerNickName, "    " + str内容, b显示退出场景按钮);
		PlayerComponent::播放声音(strPlayerNickName, "音效/BUTTON", "");
	}
	static void 玩家_虫_说(Space& refSpace, const std::string& strPlayerNickName, const std::string& str内容, const bool b显示退出场景按钮 = false)
	{
		PlayerComponent::剧情对话(refSpace, strPlayerNickName, "", "", "图片/工虫头像", "玩家：" + strPlayerNickName, "    " + str内容, b显示退出场景按钮);
		PlayerComponent::播放声音(strPlayerNickName, "音效/BUTTON", "");
	}
	static CoTask<bool> 等玩家读完(const std::string strPlayerNickName, FunCancel& funCancel)
	{
		const auto funSameSession = [&strPlayerNickName](const MyEvent::已阅读剧情对话& ref)
			{ return ref.wpPlayerGateSession.lock() == GetPlayerGateSession(strPlayerNickName).lock(); };

		auto [stop, event已阅读] = co_await CoEvent<MyEvent::已阅读剧情对话>::Wait(funCancel, __FUNCTION__, funSameSession);
		if (!event已阅读.wpPlayerGateSession.expired())
		{
			auto& refSession = *event已阅读.wpPlayerGateSession.lock();
			if (!refSession.m_wpSpace.expired())
				refSession.m_wpSpace.lock()->GetSpacePlayer(strPlayerNickName).m_msg上次发给前端的剧情对话.str对话内容.clear();
		}

		co_return stop;
	}
	CoTask<int> Co训练战(Space& refSpace, FunCancel& funCancel, const std::string strPlayerNickName)
	{
		KeepCancel kc(funCancel);

		const auto fun总教官凌云说 = [&strPlayerNickName, &refSpace](const std::string& str内容) {总教官凌云说(refSpace, strPlayerNickName, str内容); };
		const auto fun玩家说 = [&strPlayerNickName, &refSpace](const std::string& str内容) {玩家说(refSpace, strPlayerNickName, str内容); };

		fun总教官凌云说("即时战略游戏的操作并不复杂，老年间便有四句童谣：\n"
			"\t\t\t<color=#a0ff50>工程车</color>，造<color=#a0ff50>基地</color>；\n"
			"\t\t\t基地又产工程车。\n"
			"\t\t\t工程车，造<color=#a0ff50>兵营</color>，\n"
			"\t\t\t兵营产兵欢乐多！"
		); _等玩家读完returnTrue;
		fun玩家说("听说工程车还可以造<color=#a0ff50>地堡</color>和<color=#a0ff50>炮台</color>，我也要试试。"); _等玩家读完returnTrue;
		fun总教官凌云说("造完基地应该先安排工程车去采集<color=#a0ff50>晶体矿</color>和<color=#a0ff50>燃气矿</color>，这是一切生产建造的基础。此外建造<color=#a0ff50>民房</color>可以提升活动单位上限。\n加油！"); _等玩家读完returnTrue;
		fun玩家说("我只想单手操作，拖动视口 和 选中多个单位 如何操作呢？"); _等玩家读完returnTrue;
		fun总教官凌云说("\t\t拖动地面就可以移动视口，此外设置（齿轮图标）界面还有视口镜头投影切换、放大、缩小按钮。当然也支持双指缩放视口。\n"
			"\t\t先点击右边“框选”按钮，然后在屏幕中拖动，即可框选多个单位。在设置（齿轮图标）界面中可以切换菱形框选或方形框选。\n"
			"\t\t全程只要单手握持手机单指操作即可。\n"
		); _等玩家读完returnTrue;
		fun玩家说("只用一只手就能玩的RTS即时战略游戏，那岂不是跟刷短视频一样轻松？我一定要体验一下！"); _等玩家读完returnTrue;
		fun总教官凌云说("走你!"); _等玩家读完returnTrue;
		PlayerComponent::剧情对话已看完(strPlayerNickName);
		using namespace std;
		PlayerGateSession_Game::Say任务提示(strPlayerNickName, "欢迎来到<color=#a0ff50>即时战略指挥</color>，现在您要接受基础的训练");

		//auto [stop, msgResponce] = co_await AiCo::ChangeMoney(refGateSession, 100, true, funCancel);
		//if (stop)
		//{
		//	LOG(WARNING) << "ChangeMoney,协程取消";
		//	co_return 0;
		//}
		refSpace.GetSpacePlayer(strPlayerNickName).m_u32晶体矿 += 100;

		//if (co_await CoTimer::Wait(2s, funCancel))
		//	co_return 0;

		//const 单位类型 类型(单位类型::工程车);
		//单位::活动单位配置 配置;
		//单位::Find活动单位配置(类型, 配置);
		{
			auto wpSession = GetPlayerGateSession(strPlayerNickName);
			CHECK_WP_CO_RET_0(wpSession);
			auto wp视口 = wpSession.lock()->m_wp视口;
			CHECK_WP_CO_RET_0(wp视口);
			auto& ref视口 = *wp视口.lock();
			//SpEntity sp工程车 = refSpace.造活动单位(wp视口.lock()->m_upPlayer, strPlayerNickName, { 5,10 }, 配置, 类型);
			refSpace.造活动单位(ref视口, strPlayerNickName, 单位类型::工程车, { 5, 10 }, true);
			资源Component::Add(refSpace, 晶体矿, { 20, 10 });
		}
		PlayerGateSession_Game::Say任务提示(strPlayerNickName, "请点击<color=#a0ff50>建筑单位</color>=><color=#a0ff50>基地</color>”按钮，再点击空白地面，10秒后就能造出一个基地");

		const auto funSameSpace = [&refSpace, &strPlayerNickName](const MyEvent::AddEntity& refAddEntity, 单位类型 类型)
			{ return MyEvent::SameSpace(refAddEntity.wpEntity, refSpace, strPlayerNickName) && EntitySystem::Is单位类型(refAddEntity.wpEntity, 类型); };

		Position pos基地;
		{
			const auto& [stop, addEvent] = co_await CoEvent<MyEvent::AddEntity>::Wait(funCancel,"检查基地", std::bind(funSameSpace, std::placeholders::_1, 基地));
			if (stop)
				co_return 0;

			pos基地 = addEvent.wpEntity.lock()->Pos();
		}


		if (co_await CoTimer::Wait(10s, funCancel))
			co_return 0;


		PlayerGateSession_Game::Say任务提示(strPlayerNickName, "请点击<color=#a0ff50>活动单位</color>=><color=#a0ff50>工程车</color>”按钮，5秒后会在基地旁造出一个工程车");

		if (std::get<0>(co_await CoEvent<MyEvent::AddEntity>::Wait(funCancel, __FUNCTION__, std::bind(funSameSpace, std::placeholders::_1, 工程车))))
			co_return 0;

		PlayerGateSession_Game::Say任务提示(strPlayerNickName, "现在您有工程车了，点击选中您的<color=#a0ff50>工程车</color>，再点击空旷地面，命令它走向目标点");

		if (std::get<0>(co_await CoEvent<MyEvent::MoveEntity>::Wait(funCancel, __FUNCTION__, [&refSpace, &strPlayerNickName](const MyEvent::MoveEntity& ref)
			{
				if (ref.wpEntity.expired())
					return false;

				auto spEnity = ref.wpEntity.lock();
				if (&spEnity->m_refSpace != &refSpace)
					return false;

				if (EntitySystem::GetNickName(*spEnity) != strPlayerNickName)
					return false;

				if (!EntitySystem::Is单位类型(ref.wpEntity, 工程车))
					return false;

				return true;
			})))
		{
			co_return 0;
		}

		PlayerGateSession_Game::Say任务提示(strPlayerNickName, "很好！现在给您刷了一个<color=#a0ff50>晶体矿</color>，请点击晶体矿，让工程车在晶体矿和基地之间搬运晶体矿");
		资源Component::Add(refSpace, 晶体矿, { pos基地.x,pos基地.z + 20 });
		资源Component::Add(refSpace, 燃气矿, { pos基地.x + 15,pos基地.z });

		if (std::get<0>(co_await CoEvent<MyEvent::开始采集晶体矿>::Wait(funCancel)))
			co_return 0;

		PlayerGateSession_Game::Say任务提示(strPlayerNickName, "很好！您的工程车正在采集晶体矿，请等他把晶体矿运回基地。现在，请先点“取消选中”，再选中另一辆工程车去采集<color=#a0ff50>燃气矿</color>");
		if (std::get<0>(co_await CoEvent<MyEvent::晶体矿已运回基地>::Wait(funCancel)))
			co_return 0;

		PlayerGateSession_Game::Say任务提示(strPlayerNickName, "您的工程车已把第一车晶体矿运到基地，请查看左上角<color=#a0ff50>晶体矿</color>数量变化");

		if (co_await CoTimer::Wait(5s, funCancel))
			co_return 0;

		if (0 == refSpace.Get单位数(民房))
		{
			PlayerGateSession_Game::Say任务提示(strPlayerNickName, "等您存够20晶体矿后，请点击<color=#a0ff50>建筑单位</color>=><color=#a0ff50>民房</color>”按钮，再点击一次空旷地面");
			if (std::get<0>(co_await CoEvent<MyEvent::AddEntity>::Wait(funCancel, __FUNCTION__, std::bind(funSameSpace, std::placeholders::_1, 民房))))
				co_return 0;

			PlayerGateSession_Game::Say任务提示(strPlayerNickName, "很好，民房可以提升您的活动单位数量上限");
			if (co_await CoTimer::Wait(3s, funCancel))
				co_return 0;
		}


		if (0 == refSpace.Get单位数(兵营))
		{
			PlayerGateSession_Game::Say任务提示(strPlayerNickName, "等您存够30晶体矿后，请点击<color=#a0ff50>建筑单位</color>=><color=#a0ff50>兵营</color>”按钮，再点击一次空旷地面，就能造出一个兵营");
			if (std::get<0>(co_await CoEvent<MyEvent::AddEntity>::Wait(funCancel, __FUNCTION__, std::bind(funSameSpace, std::placeholders::_1, 兵营))))
				co_return 0;

			PlayerGateSession_Game::Say任务提示(strPlayerNickName, "很好，兵营可以产出<color=#a0ff50>枪兵</color>和<color=#a0ff50>近战兵</color>。");
			if (co_await CoTimer::Wait(10s, funCancel))
				co_return 0;
		}

		PlayerGateSession_Game::Say任务提示(strPlayerNickName, "请点击<color=#a0ff50>活动单位</color>=><color=#a0ff50>枪兵</color>”按钮，10秒后会在兵营旁造出一个枪兵");

		if (std::get<0>(co_await CoEvent<MyEvent::AddEntity>::Wait(funCancel, __FUNCTION__, std::bind(funSameSpace, std::placeholders::_1, 枪兵))))
			co_return 0;

		PlayerGateSession_Game::Say任务提示(strPlayerNickName, "点击选中您的枪兵，再点击地面，可以指挥他走向目标处");

		if (std::get<0>(co_await CoEvent<MyEvent::MoveEntity>::Wait(funCancel, __FUNCTION__, [&refSpace, &strPlayerNickName](const MyEvent::MoveEntity& ref) {
			return //&ref.wpEntity.lock()->m_refSpace == &refSpace; 
				MyEvent::SameSpace(ref.wpEntity, refSpace, strPlayerNickName) && EntitySystem::Is单位类型(ref.wpEntity, 枪兵);
			})))
			co_return 0;

			PlayerGateSession_Game::Say任务提示(strPlayerNickName, "现在已在左边给您刷了一个怪，控制兵走到怪附近，兵会自动打怪。您可以点右下角<color=#a0ff50>取消选中</color>，然后拖动地面看看怪在哪里");
			MonsterComponent::AddMonster(refSpace, 枪虫怪, { -30,20 });

			if (std::get<0>(co_await CoEvent<MyEvent::单位阵亡>::Wait(funCancel, __FUNCTION__, [&refSpace](const MyEvent::单位阵亡& ref) {return &ref.wpEntity.lock()->m_refSpace == &refSpace; })))
				co_return 0;

			if (0 < refSpace.Get怪物单位数())
			{
				PlayerGateSession_Game::Say任务提示(strPlayerNickName, "您的兵阵亡了。可以多造点兵去围攻敌人");

				if (std::get<0>(co_await CoEvent<MyEvent::单位阵亡>::Wait(funCancel, __FUNCTION__, [&refSpace](const MyEvent::单位阵亡& ref)
					{
						auto spEntity = ref.wpEntity.lock();
						if (&spEntity->m_refSpace != &refSpace)
							return false;

						return !spEntity->m_upPlayer;//怪物阵亡
					})))
				{
					co_return 0;
				}
			}


			PlayerGateSession_Game::Say任务提示(strPlayerNickName, "恭喜您消灭了敌人！现在左边给您刷了更多的怪。您可以造<color=#a0ff50>建筑单位</color>=><color=#a0ff50>地堡</color>”,让兵进入地堡中，立足防守，再伺机进攻");
			MonsterComponent::AddMonster(refSpace, 近战虫怪, { -30.0 }, 5);

			if (std::get<0>(co_await CoEvent<MyEvent::单位阵亡>::Wait(funCancel, __FUNCTION__, [&refSpace, &strPlayerNickName](const MyEvent::单位阵亡& ref)
				{
					auto spEntity = ref.wpEntity.lock();
					if (&spEntity->m_refSpace != &refSpace)
						return false;

					return 0 == refSpace.Get怪物单位数() || 0 == refSpace.Get玩家单位数(strPlayerNickName);
				})))
			{
				co_return 0;
			}
			if (0 == refSpace.Get怪物单位数())
			{
				PlayerComponent::播放声音(strPlayerNickName, "音效/YouWin", "您取得了胜利！您真是指挥天才！");

				fun总教官凌云说(strPlayerNickName + "，你的悟性很高，10分钟就完成了其他学员一个月才能完成的训练，为师甚是欣慰！"); _等玩家读完returnTrue;
				fun玩家说("……我这都是些什么同学啊，这么简单的操作还要学一个月吗？"); _等玩家读完returnTrue;
				fun总教官凌云说("唉，谁说不是呢！如今世风不古。遥想20年前，比你悟性更高的指挥天才比比皆是，大家经常自发研究切磋指挥技艺，更有无数学员的水平远超教官。实在令人怀念……"); _等玩家读完returnTrue;
				fun玩家说("这些前辈果真如此厉害吗？学生倒是想领教领教！"); _等玩家读完returnTrue;
				fun总教官凌云说("在“多人联机混战”和“其他人的多人战局列表”可能会遇到他们，不要轻敌。要善用战斗单位的克制关系：\n"
					"\t\t小兵 克制 坦克\n"
					"\t\t坦克 克制 地堡和炮台\n"
					"\t\t地堡和炮台 克制 步兵\n"
				); _等玩家读完returnTrue;
				fun玩家说("这些克制关系是怎么产生的呢？"); _等玩家读完returnTrue;
				fun总教官凌云说("坦克虽然伤害最高，但是价格昂贵，攻击前摇最久，移动速度最慢；<color=#a0ff50>前摇开始后炸点无法改变</color>，敌方单位很容易躲开炸点，此外坦克炸点的爆炸溅射会伤害附近的己方单位。\n"
					"\t\t近战兵移动速度快，攻击速度快，价格便宜，很容易躲开坦克炸点，也很容易在坦克攻击前摇结束之前将坦克打掉。"); _等玩家读完returnTrue;
				fun总教官凌云说("坦克，攻击距离最远，可以<color=#a0ff50>在炮台和地堡内单位的射程外攻击</color>，是拆除建筑的利器。"); _等玩家读完returnTrue;
				fun总教官凌云说("炮台价格便宜，攻击速度快，攻击距离仅次于坦克，集中放置后可对快速移动的敌方小兵群体造成有效伤害。"); _等玩家读完returnTrue;
				fun玩家说("我会在实战中体会摸索。"); _等玩家读完returnTrue;
				fun总教官凌云说("一个人的时候，可以试试<color=#a0ff50>防守战</color>，有助于体会炮台、坦克、地堡的特点。也可以试试<color=#a0ff50>攻坚战</color>，练习步兵与坦克的配合"); _等玩家读完returnTrue;
				玩家说(refSpace, strPlayerNickName, "好的。", true);//	_等玩家读完;
			}
			else
			{
				PlayerComponent::播放声音(strPlayerNickName, "音效/YouLoss", "训练失败。");

				fun总教官凌云说("这只是一次训练，不必过于在意。失败是成功之母，待情绪平复可以再试一次。"); _等玩家读完returnTrue;
				玩家说(refSpace, strPlayerNickName, "好的。", true);//	_等玩家读完;
			}
			co_return 0;
	}
	CoTask<int> Co训练战_虫(Space& refSpace, FunCancel& funCancel, const std::string strPlayerNickName)
	{
		KeepCancel kc(funCancel);

		const auto fun房虫胡噜说 = [&strPlayerNickName, &refSpace](const std::string& str内容) {房虫胡噜说(refSpace, strPlayerNickName, str内容); };
		const auto fun玩家说 = [&strPlayerNickName, &refSpace](const std::string& str内容) {玩家_虫_说(refSpace, strPlayerNickName, str内容); };

		fun房虫胡噜说("你醒了?"); _等玩家读完returnTrue;
		fun玩家说("我们到家了吗？"); _等玩家读完returnTrue;
		fun房虫胡噜说("是的，我们回到地球了。但是地球已被人类占据。"); _等玩家读完returnTrue;
		fun玩家说("我们仅仅离开了6000万年而已，想不到啊……一定要夺回我们的家园！"); _等玩家读完returnTrue;
		fun房虫胡噜说("你睡得太久，需要一些恢复训练才能投入战斗！"); _等玩家读完returnTrue;
		fun玩家说("好的。"); _等玩家读完returnTrue;
		PlayerComponent::剧情对话已看完(strPlayerNickName);
		using namespace std;

		refSpace.GetSpacePlayer(strPlayerNickName).m_u32晶体矿 += 100;
		{
			auto wpSession = GetPlayerGateSession(strPlayerNickName);
			CHECK_WP_CO_RET_0(wpSession);
			auto wp视口 = wpSession.lock()->m_wp视口;
			CHECK_WP_CO_RET_0(wp视口);
			auto& ref视口 = *wp视口.lock();
			//SpEntity sp工程车 = refSpace.造活动单位(wp视口.lock()->m_upPlayer, strPlayerNickName, { 5,10 }, 配置, 类型);
			refSpace.造活动单位(ref视口, strPlayerNickName, 单位类型::工虫, { 5, 10 }, true);
			资源Component::Add(refSpace, 晶体矿, { 20, 10 });
		}
		PlayerGateSession_Game::Say任务提示(strPlayerNickName, "请点击<color=#a0ff50>建筑单位</color>=><color=#a0ff50>虫巢</color>”按钮，再点击空白地面，工虫会在10秒内变异为虫巢");

		const auto funSameSpace = [&refSpace, &strPlayerNickName](const MyEvent::AddEntity& refAddEntity, 单位类型 类型)
			{ return MyEvent::SameSpace(refAddEntity.wpEntity, refSpace, strPlayerNickName) && EntitySystem::Is单位类型(refAddEntity.wpEntity, 类型); };

		Position pos基地;
		{
			const auto& [stop, addEvent] = co_await CoEvent<MyEvent::AddEntity>::Wait(funCancel, __FUNCTION__, std::bind(funSameSpace, std::placeholders::_1, 虫巢));
			if (stop)
				co_return 0;

			pos基地 = addEvent.wpEntity.lock()->Pos();
		}


		if (co_await CoTimer::Wait(10s, funCancel))
			co_return 0;


		PlayerGateSession_Game::Say任务提示(strPlayerNickName,
			"很好。虫巢周围会形成<color=#a0ff50>苔蔓(wàn)</color>，苔蔓上的虫类单位移动速度会变快，并且回血速度也会加快。应该<color=#a0ff50>尽量在苔蔓上与敌方单位交战</color>。"
			"等虫巢产出<color=#a0ff50>幼虫</color>后，请点击<color=#a0ff50>活动单位</color>=><color=#a0ff50>工虫</color>”按钮，让幼虫蜕变为工虫");

		if (std::get<0>(co_await CoEvent<MyEvent::AddEntity>::Wait(funCancel, __FUNCTION__, std::bind(funSameSpace, std::placeholders::_1, 工虫))))
			co_return 0;

		PlayerGateSession_Game::Say任务提示(strPlayerNickName, "现在您有工虫了，点击选中您的<color=#a0ff50>工虫</color>，再点击空旷地面，命令它走向目标点");

		if (std::get<0>(co_await CoEvent<MyEvent::MoveEntity>::Wait(funCancel, __FUNCTION__, [&refSpace, &strPlayerNickName](const MyEvent::MoveEntity& ref)
			{
				if (ref.wpEntity.expired())
					return false;

				auto spEnity = ref.wpEntity.lock();
				if (&spEnity->m_refSpace != &refSpace)
					return false;

				if (EntitySystem::GetNickName(*spEnity) != strPlayerNickName)
					return false;

				if (!EntitySystem::Is单位类型(ref.wpEntity, 工虫))
					return false;

				return true;
			})))
		{
			co_return 0;
		}

		PlayerGateSession_Game::Say任务提示(strPlayerNickName, "很好！现在给您刷了一个晶体矿，请点击选中<color=#a0ff50>工虫</color>，再点击一次<color=#a0ff50>晶体矿</color>，让工虫在晶体矿和虫巢之间搬运晶体矿");
		资源Component::Add(refSpace, 晶体矿, { pos基地.x,pos基地.z + 20 });
		资源Component::Add(refSpace, 燃气矿, { pos基地.x + 15,pos基地.z });

		if (std::get<0>(co_await CoEvent<MyEvent::开始采集晶体矿>::Wait(funCancel)))
			co_return 0;

		PlayerGateSession_Game::Say任务提示(strPlayerNickName, "很好！您的工虫正在采集晶体矿，请等他把晶体矿运回基地。现在，请先点“取消选中”，再蜕变一个工虫去采集<color=#a0ff50>燃气矿</color>");
		if (std::get<0>(co_await CoEvent<MyEvent::晶体矿已运回基地>::Wait(funCancel)))
			co_return 0;

		PlayerGateSession_Game::Say任务提示(strPlayerNickName, "您的工虫已把第一车晶体矿运到基地，请查看<color=#a0ff50>左上角</color>晶体矿数量变化");

		if (co_await CoTimer::Wait(5s, funCancel))
			co_return 0;

		if (0 == refSpace.Get单位数(房虫))
		{
			PlayerGateSession_Game::Say任务提示(strPlayerNickName, "等您存够20晶体矿后，请点击<color=#a0ff50>活动单位</color>=><color=#a0ff50>房虫</color>”按钮，让幼虫蜕变为房虫");
			if (std::get<0>(co_await CoEvent<MyEvent::AddEntity>::Wait(funCancel, __FUNCTION__, std::bind(funSameSpace, std::placeholders::_1, 房虫))))
				co_return 0;

			PlayerGateSession_Game::Say任务提示(strPlayerNickName, "很好，房虫可以提升您的<color=#a0ff50>活动单位数量上限</color>");
			if (co_await CoTimer::Wait(3s, funCancel))
				co_return 0;
		}

		if (0 == refSpace.Get单位数(虫营))
		{
			PlayerGateSession_Game::Say任务提示(strPlayerNickName, "等您存够30晶体矿后，请点击<color=#a0ff50>建筑单位</color>=><color=#a0ff50>虫营</color>按钮，再点击覆盖有苔蔓(wàn)的地面，就能变异出一个虫营。");
			if (std::get<0>(co_await CoEvent<MyEvent::AddEntity>::Wait(funCancel, __FUNCTION__, std::bind(funSameSpace, std::placeholders::_1, 虫营))))
				co_return 0;

			PlayerGateSession_Game::Say任务提示(strPlayerNickName, "很好，虫营可以解锁枪虫和近战虫。虫营只要一个就够了，如果希望出兵更快，应该造更多的虫巢。");
			if (co_await CoTimer::Wait(10s, funCancel))
				co_return 0;
		}

		PlayerGateSession_Game::Say任务提示(strPlayerNickName, "请点击<color=#a0ff50>活动单位</color>=><color=#a0ff50>近战虫</color>按钮，让幼虫蜕变为近战虫");

		if (std::get<0>(co_await CoEvent<MyEvent::AddEntity>::Wait(funCancel, __FUNCTION__, std::bind(funSameSpace, std::placeholders::_1, 近战虫))))
			co_return 0;

		PlayerGateSession_Game::Say任务提示(strPlayerNickName, "点击选中您的<color=#a0ff50>近战虫</color>，再点击地面，可以指挥它走向目标处");

		if (std::get<0>(co_await CoEvent<MyEvent::MoveEntity>::Wait(funCancel, __FUNCTION__, [&refSpace, &strPlayerNickName](const MyEvent::MoveEntity& ref) {
			return //&ref.wpEntity.lock()->m_refSpace == &refSpace; 
				MyEvent::SameSpace(ref.wpEntity, refSpace, strPlayerNickName) && EntitySystem::Is单位类型(ref.wpEntity, 近战虫);
			})))
			co_return 0;

			PlayerGateSession_Game::Say任务提示(strPlayerNickName, "现在已在左边给您刷了一个敌人，点击选中您的<color=#a0ff50>近战虫</color>，再<color=#a0ff50>点击敌人附近的地面</color>，近战虫会走向目标并自动攻击。您可以点右下角“取消选中”然后拖动地面看看敌人在哪里");
			MonsterComponent::AddMonster(refSpace, 枪兵怪, { -30,20 });

			if (std::get<0>(co_await CoEvent<MyEvent::单位阵亡>::Wait(funCancel, __FUNCTION__, [&refSpace](const MyEvent::单位阵亡& ref) {return &ref.wpEntity.lock()->m_refSpace == &refSpace; })))
				co_return 0;

			if (0 < refSpace.Get怪物单位数())
			{
				PlayerGateSession_Game::Say任务提示(strPlayerNickName, "您的单位阵亡了。可以多造点近战虫和枪虫去围攻敌人");

				if (std::get<0>(co_await CoEvent<MyEvent::单位阵亡>::Wait(funCancel, __FUNCTION__, [&refSpace](const MyEvent::单位阵亡& ref)
					{
						auto spEntity = ref.wpEntity.lock();
						if (&spEntity->m_refSpace != &refSpace)
							return false;

						return !spEntity->m_upPlayer;//怪物阵亡
					})))
				{
					co_return 0;
				}
			}


			PlayerGateSession_Game::Say任务提示(strPlayerNickName, "恭喜您消灭了敌人！现在左边给您刷了更多的敌人。您可以变异更多的<color=#a0ff50>虫巢</color>加快幼虫的产出，蜕变更多的<color=#a0ff50>房虫</color>提升活动单位上限，加油");
			MonsterComponent::AddMonster(refSpace, 近战兵怪, { -30.0 }, 3);

			if (std::get<0>(co_await CoEvent<MyEvent::单位阵亡>::Wait(funCancel, __FUNCTION__, [&refSpace, &strPlayerNickName](const MyEvent::单位阵亡& ref)
				{
					auto spEntity = ref.wpEntity.lock();
					if (&spEntity->m_refSpace != &refSpace)
						return false;

					return 0 == refSpace.Get怪物单位数() || 0 == refSpace.Get玩家单位数(strPlayerNickName);
				})))
			{
				co_return 0;
			}
			if (0 == refSpace.Get怪物单位数())
			{
				PlayerComponent::播放声音(strPlayerNickName, "音效/YouWin", "您取得了胜利！");

				fun房虫胡噜说(strPlayerNickName + "，很好，夺回地球指日可待！"); _等玩家读完returnTrue;
				玩家_虫_说(refSpace, strPlayerNickName, "期待与人类的实战。", true);//	_等玩家读完;
			}
			else
			{
				PlayerComponent::播放声音(strPlayerNickName, "音效/YouLoss", "训练失败。");

				fun房虫胡噜说("这只是一次训练，不必过于在意。失败是成功之母，待情绪平复可以再试一次。"); _等玩家读完returnTrue;
				玩家说(refSpace, strPlayerNickName, "好的。", true);//	_等玩家读完;
			}
			co_return 0;
	}
	Position 怪物走向矿附近(const Position& refOld)
	{
		return { -30,30 };
	}

	static CoTask<bool> Is战斗结束(Space& refSpace, const std::string strPlayerNickName, FunCancel& funCancel)
	{
		const auto fun总教官凌云说 = [&strPlayerNickName, &refSpace](const std::string& str内容) {总教官凌云说(refSpace, strPlayerNickName, str内容); };
		const auto fun玩家说 = [&strPlayerNickName, &refSpace](const std::string& str内容) {玩家说(refSpace, strPlayerNickName, str内容); };

		if (0 == refSpace.Get怪物单位数())
		{
			PlayerGateSession_Game::播放声音(strPlayerNickName, "音效/YouWin", "您守住了！您真是指挥天才！");

			fun总教官凌云说(strPlayerNickName + "，我果然没有看错你，你是一个指挥天才！"); _等玩家读完returnTrue;
			fun玩家说("承蒙教官谬赞。学生谨遵教诲，自不敢有一丝懈怠，定当不负所望，继续精进。");	_等玩家读完returnTrue;
			PlayerComponent::剧情对话(refSpace, strPlayerNickName, "图片/女教官白色海军服", "总教官：凌云", "", "", "    走你!", true);
			co_return true;
		}
		if (0 == refSpace.Get玩家单位数(strPlayerNickName))
		{
			PlayerGateSession_Game::播放声音(strPlayerNickName, "音效/YouLose", "胜败乃兵家常事，请点击右上角“退出场景”离开，然后再次点击“防守战”，就可以重新来过。");
			fun总教官凌云说("胜败乃兵家常事，我仍然看好你的潜力！"); _等玩家读完returnTrue;
			玩家说(refSpace, strPlayerNickName, "学生此次功败垂成，定会吸取教训，总结经验，再次努力，定不辜负教官的栽培与期望。", true);//	_等玩家读完;
			co_return true;
		}
		co_return false;
	}
	static CoTask<bool> Is战斗结束_虫(Space& refSpace, const std::string strPlayerNickName, FunCancel& funCancel)
	{
		const auto fun房虫胡噜说 = [&strPlayerNickName, &refSpace](const std::string& str内容) {房虫胡噜说(refSpace, strPlayerNickName, str内容); };
		const auto fun玩家说 = [&strPlayerNickName, &refSpace](const std::string& str内容) {玩家_虫_说(refSpace, strPlayerNickName, str内容); };

		if (0 == refSpace.Get怪物单位数())
		{
			PlayerGateSession_Game::播放声音(strPlayerNickName, "音效/YouWin", "您守住了！您真是指挥天才！");

			fun房虫胡噜说(strPlayerNickName + "，我果然没有看错你，你是一个指挥天才！"); _等玩家读完returnTrue;
			fun玩家说("承蒙谬赞。学生谨遵教诲，自不敢有一丝懈怠，定当不负所望，继续精进。");	_等玩家读完returnTrue;
			PlayerComponent::剧情对话(refSpace, strPlayerNickName, "图片/房虫头像", "房虫：胡噜", "", "", "    走你!", true);
			co_return true;
		}
		if (0 == refSpace.Get玩家单位数(strPlayerNickName))
		{
			PlayerGateSession_Game::播放声音(strPlayerNickName, "音效/YouLose", "胜败乃兵家常事，请点击右上角“退出场景”离开，就可以重新来过。");
			fun房虫胡噜说("胜败乃兵家常事，我仍然看好你的潜力！"); _等玩家读完returnTrue;
			玩家_虫_说(refSpace, strPlayerNickName, "学生此次功败垂成，定会吸取教训，总结经验，再次努力，定不辜负教官的栽培与期望。", true);//	_等玩家读完;
			co_return true;
		}
		co_return false;
	}
	CoTask<int> Co防守战_虫(Space& refSpace, FunCancel& funCancel, const std::string strPlayerNickName)
	{
		KeepCancel kc(funCancel);
		using namespace std;

		const auto fun房虫胡噜说 = [&strPlayerNickName, &refSpace](const std::string& str内容) {房虫胡噜说(refSpace, strPlayerNickName, str内容); };
		const auto fun玩家说 = [&strPlayerNickName, &refSpace](const std::string& str内容) {玩家_虫_说(refSpace, strPlayerNickName, str内容); };

		fun房虫胡噜说("情报显示：敌人已发现我们的巢穴入口，即将入侵，请做好准备。");	_等玩家读完returnTrue;
		fun玩家说("可是我仅受过简单的指挥训练。");							_等玩家读完returnTrue;
		fun房虫胡噜说("我们会提供足够的初始晶体矿和燃气矿，你<color=#a0ff50>不用再从零开始采集</color>。");	_等玩家读完returnTrue;
		fun玩家说("我还是没有把握。");										_等玩家读完returnTrue;
		fun房虫胡噜说("我会传授你指挥实战速成口诀，三分钟即可成为实战高手。");	_等玩家读完returnTrue;
		fun玩家说("真有这种口诀的话就应该在训练战时告诉我啊！");			_等玩家读完returnTrue;
		fun房虫胡噜说("实战口诀只有在九死一生的实战中才有用！");		_等玩家读完returnTrue;
		fun玩家说("九死一生？形势严峻啊，我会全力以赴的!");					_等玩家读完returnTrue;
		fun房虫胡噜说("1个虫巢只能照顾<color=#a0ff50>3只幼虫</color>。<color=#a0ff50>让工虫变异更多的虫巢</color>，才有更多的幼虫可供蜕变。<color=#a0ff50>多蜕变房虫</color>，增加活动单位上限。"); _等玩家读完returnTrue;
		fun玩家说("就是一定要在数量上占据优势吗？");	_等玩家读完returnTrue;
		fun房虫胡噜说("是的，进攻就是最好的防守。派出地面单位堵住敌人的通道，再派出飞虫越过障碍物空袭敌人。"); _等玩家读完returnTrue;
		fun玩家说("OK，也不是很复杂。我有信心完成任务！");					_等玩家读完returnTrue;
		fun房虫胡噜说("走你！");										_等玩家读完returnTrue;

		PlayerComponent::剧情对话已看完(strPlayerNickName);

		PlayerGateSession_Game::Say任务提示(strPlayerNickName, "防守战：只要守住，就是胜利！");

		refSpace.GetSpacePlayer(strPlayerNickName).m_u32燃气矿 += 1000;
		refSpace.GetSpacePlayer(strPlayerNickName).m_u32晶体矿 += 2000;

		auto wpSession = GetPlayerGateSession(strPlayerNickName);
		CHECK_WP_CO_RET_0(wpSession);
		auto wp视口 = wpSession.lock()->m_wp视口;
		CHECK_WP_CO_RET_0(wp视口);

		{
			refSpace.造活动单位(*wp视口.lock(), strPlayerNickName, 单位类型::工虫, { -15, 15 });
			refSpace.造活动单位(*wp视口.lock(), strPlayerNickName, 单位类型::工虫, { -15, 20 });
			refSpace.造活动单位(*wp视口.lock(), strPlayerNickName, 单位类型::工虫, { -20, 15 });
			refSpace.造活动单位(*wp视口.lock(), strPlayerNickName, 单位类型::工虫, { -20, 20 });
			refSpace.造活动单位(*wp视口.lock(), strPlayerNickName, 单位类型::工虫, { -30, 30 });

			资源Component::Add(refSpace, 晶体矿, { -20, 35 });
			资源Component::Add(refSpace, 晶体矿, { -26, 20 });
			资源Component::Add(refSpace, 晶体矿, { -32, 20 });
			资源Component::Add(refSpace, 燃气矿, { -29, 35 });
			资源Component::Add(refSpace, 燃气矿, { -20, 20 });
			资源Component::Add(refSpace, 燃气矿, { -29, 25 });
		}

		PlayerGateSession_Game::Say任务提示(strPlayerNickName, "5秒后将出现第1波敌人。多造虫巢和房虫，多造近战虫枪虫和飞虫，一定要在数量上压倒敌人！");
		if (co_await CoTimer::Wait(5s, funCancel))
			co_return 0;

		PlayerGateSession_Game::Say任务提示(strPlayerNickName, "<color=#a0ff50>前期不用采集资源</color>。多造虫巢和房虫，多造近战虫枪虫和飞虫，一定要在数量上压制敌人！");

		for (int i = 1; i < 20; ++i)
		{
			if (1 < i && co_await Is战斗结束_虫(refSpace, strPlayerNickName, funCancel))
				co_return 0;

			PlayerGateSession_Game::Say系统(strPlayerNickName, std::format("第{0}波敌人正向您走来", i));
			auto vecEneity = MonsterComponent::AddMonster(refSpace, i % 2 == 0 ? 枪兵怪 : 近战兵怪, { 48,-48 }, i * 1);
			auto vecEneity2 = MonsterComponent::AddMonster(refSpace, 工程车怪, { 47,-47 }, i * 1);
			vecEneity.insert(vecEneity.end(), vecEneity2.begin(), vecEneity2.end());
			for (auto& spEntity : vecEneity)
			{
				if (spEntity->m_up找目标走过去)
					spEntity->m_up找目标走过去->m_fun空闲走向此处 = 怪物走向矿附近;
			}

			if (co_await CoTimer::Wait(25s, funCancel))
				co_return 0;
		}

		while (!co_await CoTimer::Wait(5s, funCancel))
		{
			if (co_await Is战斗结束_虫(refSpace, strPlayerNickName, funCancel))
				co_return 0;
		}
		co_return 0;
	}
	CoTask<int> Co防守战(Space& refSpace, FunCancel& funCancel, const std::string strPlayerNickName)
	{
		KeepCancel kc(funCancel);
		using namespace std;

		const auto fun总教官凌云说 = [&strPlayerNickName, &refSpace](const std::string& str内容) {总教官凌云说(refSpace, strPlayerNickName, str内容); };
		const auto fun玩家说 = [&strPlayerNickName, &refSpace](const std::string& str内容) {玩家说(refSpace, strPlayerNickName, str内容); };

		fun总教官凌云说("情报显示：将有大量敌方单位进攻我方基地，请做好准备。");	_等玩家读完returnTrue;
		fun玩家说("可是我仅受过简单的指挥训练。");							_等玩家读完returnTrue;
		fun总教官凌云说("我们会提供足够的初始晶体矿和燃气矿，你不用再从零开始采集。");	_等玩家读完returnTrue;
		fun玩家说("我还是没有把握。");										_等玩家读完returnTrue;
		fun总教官凌云说("我会传授你指挥实战速成口诀，三分钟即可成为实战高手。");	_等玩家读完returnTrue;
		fun玩家说("真有这种口诀的话就应该在训练战时告诉我啊！");			_等玩家读完returnTrue;
		fun总教官凌云说("实战口诀只有在九死一生的实战中才有用！");		_等玩家读完returnTrue;
		fun玩家说("九死一生？形势严峻啊，我会全力以赴的!");					_等玩家读完returnTrue;
		fun总教官凌云说("<color=#a0ff50>坦克</color>可以超远距离造成超高伤害，而且是范围伤害，但是坦克锁定炮弹落点后有超长前摇时长，前摇结束后，敌方目标可能已经远离炮弹落点。炮弹落点附近的我方单位也会受到伤害。"); _等玩家读完returnTrue;
		fun玩家说("有点鸡肋的感觉，不是红警中的高速坦克，与星际中的攻城坦克也有些不同。");	_等玩家读完returnTrue;
		fun总教官凌云说("…不知道你在说什么…有一种叫<color=#a0ff50>炮台</color>的建筑单位，攻击距离很远，仅次于坦克；前摇时长很短，可以快速攻击；缺陷是伤害较低；不过多造点集中防守某一块区域还是很实用的。"); _等玩家读完returnTrue;
		fun玩家说("看来<color=#a0ff50>只要有坦克和炮台就能守住</color>对吗？");					_等玩家读完returnTrue;
		fun总教官凌云说("是的，这是其他指挥官总结的经验。大概<color=#a0ff50>10个炮台和5辆坦克沿墙边布置就能守住</color>。当然还有其他多种可行的防守方法，要靠你自行摸索。"); _等玩家读完returnTrue;
		fun玩家说("OK，也不是很复杂。我有信心完成任务！");					_等玩家读完returnTrue;
		fun总教官凌云说("走你！");										_等玩家读完returnTrue;

		PlayerComponent::剧情对话已看完(strPlayerNickName);

		PlayerGateSession_Game::Say任务提示(strPlayerNickName, "防守战：只要守住，就是胜利！");
		//if (co_await CoTimer::Wait(1s, funCancel))
		//	co_return 0;

		refSpace.GetSpacePlayer(strPlayerNickName).m_u32燃气矿 += 300;
		refSpace.GetSpacePlayer(strPlayerNickName).m_u32晶体矿 += 1000;
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
		auto wpSession = GetPlayerGateSession(strPlayerNickName);
		CHECK_WP_CO_RET_0(wpSession);
		auto wp视口 = wpSession.lock()->m_wp视口;
		CHECK_WP_CO_RET_0(wp视口);

		{
			refSpace.造活动单位(*wp视口.lock(), strPlayerNickName, 单位类型::工程车, { -15, 15 });


			资源Component::Add(refSpace, 晶体矿, { -20, 35 });
			资源Component::Add(refSpace, 晶体矿, { -26, 20 });
			资源Component::Add(refSpace, 燃气矿, { -29, 35 });
			资源Component::Add(refSpace, 燃气矿, { -20, 20 });

		}

		PlayerGateSession_Game::Say任务提示(strPlayerNickName, "5秒后将出现第1波怪。您可以沿右边墙边造炮台、布置坦克、造地堡、让兵进驻地堡。");
		if (co_await CoTimer::Wait(5s, funCancel))
			co_return 0;

		PlayerGateSession_Game::Say任务提示(strPlayerNickName, "<color=#a0ff50>前期不用采集资源</color>。沿右边迷宫出口墙边布置一排<color=#a0ff50>炮台</color>，再放置<color=#a0ff50>坦克</color>加强防御。");

		for (int i = 1; i < 20; ++i)
		{
			if (1 < i && co_await Is战斗结束(refSpace, strPlayerNickName, funCancel))
				co_return 0;

			PlayerGateSession_Game::Say系统(strPlayerNickName, std::format("第{0}波敌人正向您走来", i));
			auto vecEneity = MonsterComponent::AddMonster(refSpace, i % 2 == 0 ? 枪虫怪 : 近战虫怪, { 48,-48 }, i * 1);
			auto vecEneity2 = MonsterComponent::AddMonster(refSpace, 工虫怪, { 47,-47 }, i * 1);
			vecEneity.insert(vecEneity.end(), vecEneity2.begin(), vecEneity2.end());
			for (auto& spEntity : vecEneity)
			{
				if (spEntity->m_up找目标走过去)
					spEntity->m_up找目标走过去->m_fun空闲走向此处 = 怪物走向矿附近;
			}

			if (co_await CoTimer::Wait(25s, funCancel))
				co_return 0;
		}

		while (!co_await CoTimer::Wait(5s, funCancel))
		{
			if (co_await Is战斗结束(refSpace, strPlayerNickName, funCancel))
				co_return 0;
		}
		co_return 0;
	}

	static WpEntity 创建敌方建筑(Space& refSpace, const Position& pos, const 单位类型 类型)
	{
		auto wp = 造建筑Component::创建建筑(refSpace, pos, 类型, {}, "");
		CHECK_WP_RET_DEFAULT(wp);
		auto& refEntity = *wp.lock();
		refEntity.m_upBuilding->直接造好();
		return wp;
	}

	static CoTask<bool> 攻坚战胜利(Space& refSpace, const std::string strPlayerNickName, FunCancel& funCancel)
	{
		const auto fun坦克手齐诺维说 = [&strPlayerNickName, &refSpace](const std::string& str内容) {坦克手齐诺维说(refSpace, strPlayerNickName, str内容); };
		const auto fun玩家说 = [&strPlayerNickName, &refSpace](const std::string& str内容) {玩家说(refSpace, strPlayerNickName, str内容); };
		fun坦克手齐诺维说("总算报仇了！");	_等玩家读完returnTrue;
		fun玩家说("以后坦克不能再单独行动了，一定要带上步兵。");	_等玩家读完returnTrue;
		fun坦克手齐诺维说("是，指挥官！");	_等玩家读完returnTrue;
		玩家说(refSpace, strPlayerNickName, "走！", true);
		co_return false;
	}
	static CoTask<bool> 攻坚战胜利_虫(Space& refSpace, const std::string strPlayerNickName, FunCancel& funCancel)
	{
		//const auto fun房虫胡噜说 = [&strPlayerNickName, &refSpace](const std::string& str内容) {房虫胡噜说(refSpace, strPlayerNickName, str内容); };
		const auto fun玩家说 = [&strPlayerNickName, &refSpace](const std::string& str内容) {玩家_虫_说(refSpace, strPlayerNickName, str内容); };
		const auto fun绿色坦克洪隆说 = [&strPlayerNickName, &refSpace](const std::string& str内容) {绿色坦克洪隆说(refSpace, strPlayerNickName, str内容); };
		fun绿色坦克洪隆说("总算报仇了！");	_等玩家读完returnTrue;
		fun玩家说("以后坦克不能再单独行动了，一定要带上步兵。");	_等玩家读完returnTrue;
		fun绿色坦克洪隆说("是，指挥官！");	_等玩家读完returnTrue;
		玩家_虫_说(refSpace, strPlayerNickName, "走！", true);
		co_return false;
	}

	static CoTask<bool> 攻坚战失败(Space& refSpace, const std::string strPlayerNickName, FunCancel& funCancel, const bool b已营救坦克连)
	{
		const auto fun装甲指挥官海因茨说 = [&strPlayerNickName, &refSpace](const std::string& str内容) {装甲指挥官海因茨说(refSpace, strPlayerNickName, str内容); };
		const auto fun坦克手齐诺维说 = [&strPlayerNickName, &refSpace](const std::string& str内容) {坦克手齐诺维说(refSpace, strPlayerNickName, str内容); };
		const auto fun玩家说 = [&strPlayerNickName, &refSpace](const std::string& str内容) {玩家说(refSpace, strPlayerNickName, str内容); };

		if (b已营救坦克连)
		{
			fun坦克手齐诺维说("早知如此就该听你的直接回去！"); _等玩家读完returnTrue;
			玩家说(refSpace, strPlayerNickName, "现在说什么都晚了（惨叫）。", true);
		}
		else
		{
			fun装甲指挥官海因茨说("可以试试操作慢一点，先进攻<color=#a0ff50>右下角的虫巢</color>。"); _等玩家读完returnTrue;
			玩家说(refSpace, strPlayerNickName, "只能下次再试试了。", true);
		}
		co_return false;
	}
	static CoTask<bool> 攻坚战失败_虫(Space& refSpace, const std::string strPlayerNickName, FunCancel& funCancel, const bool b已营救坦克连)
	{
		const auto fun房虫胡噜说 = [&strPlayerNickName, &refSpace](const std::string& str内容) {房虫胡噜说(refSpace, strPlayerNickName, str内容); };
		const auto fun绿色坦克洪隆说 = [&strPlayerNickName, &refSpace](const std::string& str内容) {绿色坦克洪隆说(refSpace, strPlayerNickName, str内容); };

		if (b已营救坦克连)
		{
			fun绿色坦克洪隆说("早知如此就该听你的直接回去！"); _等玩家读完returnTrue;
			玩家_虫_说(refSpace, strPlayerNickName, "现在说什么都晚了（惨叫）。", true);
		}
		else
		{
			fun房虫胡噜说("可以试试操作慢一点，先进攻右下角的虫巢。"); _等玩家读完returnTrue;
			玩家_虫_说(refSpace, strPlayerNickName, "只能下次再试试了。", true);
		}
		co_return false;
	}
	CoTask<int> Co攻坚战(Space& refSpace, FunCancel& funCancel, const std::string strPlayerNickName)
	{
		KeepCancel kc(funCancel);

		const auto fun装甲指挥官海因茨说 = [&strPlayerNickName, &refSpace](const std::string& str内容) {装甲指挥官海因茨说(refSpace, strPlayerNickName, str内容); };
		const auto fun科学家玛丽亚说 = [&strPlayerNickName, &refSpace](const std::string& str内容) {科学家玛丽亚说(refSpace, strPlayerNickName, str内容); };
		const auto fun坦克手齐诺维说 = [&strPlayerNickName, &refSpace](const std::string& str内容) {坦克手齐诺维说(refSpace, strPlayerNickName, str内容); };
		const auto fun玩家说 = [&strPlayerNickName, &refSpace](const std::string& str内容) {玩家说(refSpace, strPlayerNickName, str内容); };

		fun装甲指挥官海因茨说(strPlayerNickName + "，我的一个坦克连在调查<color=#a0ff50>虫巢</color>时失联，请前往营救。"); _等玩家读完returnTrue;
		fun玩家说("坦克没有步兵保护吗？");	_等玩家读完returnTrue;
		fun装甲指挥官海因茨说("是的，步兵刚刚到位，两个班的兵力，现在全部交给你指挥。"); _等玩家读完returnTrue;
		fun玩家说("虫巢是什么？");	_等玩家读完returnTrue;
		fun科学家玛丽亚说("侦察机传回的视频表明：虫巢周围会形成<color=#a0ff50>苔蔓(wàn)</color>，苔蔓上的虫类单位移动速度会变快，并且回血速度也会加快。应该<color=#a0ff50>避免在苔蔓上与虫类单位交战</color>。"
			"\n\n据1985年《普通话异读词审音表》：\n苔蔓(<color=#a0ff50>wàn</color>)、蔓(màn)延");	_等玩家读完returnTrue;
		fun玩家说("一个坦克连应该有10辆坦克吧？这么多钢铁疙瘩能凭空消失吗？");	_等玩家读完returnTrue;
		fun装甲指挥官海因茨说("现在人手紧缺，我不能做任何行动上的承诺。但我个人会竭尽全力，不放弃任何一个人。"); _等玩家读完returnTrue;
		fun玩家说("我会尽力而为的。");	_等玩家读完returnTrue;
		fun装甲指挥官海因茨说("要多用脑子，要多想。记住兵种克制关系：\n<color=#a0ff50>小兵 克制 坦克\n坦克 克制 炮台\n炮台 克制 小兵</color>"); _等玩家读完returnTrue;
		fun玩家说("这我早就记住了，否则我也不会疑惑为什么坦克没有步兵保护。");	_等玩家读完returnTrue;
		fun装甲指挥官海因茨说("我们相信你的能力，你一定能平安归来!"); _等玩家读完returnTrue;
		PlayerComponent::剧情对话已看完(strPlayerNickName);

		{
			{
				auto wpSession = GetPlayerGateSession(strPlayerNickName);
				CHECK_WP_CO_RET_0(wpSession);
				auto wp视口 = wpSession.lock()->m_wp视口;
				CHECK_WP_CO_RET_0(wp视口);
				auto& ref视口 = *wp视口.lock();

				refSpace.造活动单位(ref视口, strPlayerNickName, 单位类型::近战兵, { -40, 10 }, true);
				for (int i = 0; i < 8; ++i)
				{
					const float z = 10.f + i * 5;
					refSpace.造活动单位(ref视口, strPlayerNickName, 单位类型::枪兵, { -40, z });
					refSpace.造活动单位(ref视口, strPlayerNickName, 单位类型::近战兵, { -45, z });
				}
			}
			for (int i = 0; i < 3; ++i)
			{
				MonsterComponent::AddMonster(refSpace, 三色坦克, { 40,-49.f + i * 5 }, 1);
				MonsterComponent::AddMonster(refSpace, 三色坦克, { -40,-49.f + i * 5 }, 1);
			}
			for (int i = 0; i < 5; ++i)
			{
				创建敌方建筑(refSpace, { 25,-49.f + i * 5 }, 炮台);
				创建敌方建筑(refSpace, { 30,-49.f + i * 5 }, 炮台);
				//创建敌方建筑(refSpace, { 35,-49.f + i * 5 }, 炮台);

				创建敌方建筑(refSpace, { -25,-49.f + i * 5 }, 炮台);
				创建敌方建筑(refSpace, { -30,-49.f + i * 5 }, 炮台);
				//创建敌方建筑(refSpace, { -35,-49.f + i * 5 }, 炮台);
			}

			{
				//守卫右下角虫巢的怪
				MonsterComponent::AddMonster(refSpace, 枪虫怪, { 10,35.f }, 2);
				MonsterComponent::AddMonster(refSpace, 近战虫怪, { 10,49.f }, 2);
			}

			auto wp虫巢右下 = 创建敌方建筑(refSpace, { 10, 40.f }, 虫巢);
			创建敌方建筑(refSpace, { -45, -45.f }, 虫巢);
			创建敌方建筑(refSpace, { 45, -45.f }, 虫巢);

			PlayerGateSession_Game::Say任务提示(strPlayerNickName, "此局没有工程车。<color=#a0ff50>右下方的敌方虫巢防守薄弱</color>。");
			bool b已营救坦克连 = false;
			while (true)
			{
				auto [stop, responce] = co_await CoEvent<MyEvent::单位阵亡>::Wait(funCancel, __FUNCTION__, [&refSpace, wp虫巢右下](const MyEvent::单位阵亡& ref) {return &ref.wpEntity.lock()->m_refSpace == &refSpace; });
				if (stop)
					co_return 0;

				if (0 == refSpace.Get怪物单位数(虫巢))
				{
					co_await 攻坚战胜利(refSpace, strPlayerNickName, funCancel);
					co_return 0;
				}

				if (0 == refSpace.Get玩家单位数(strPlayerNickName))
				{
					co_await 攻坚战失败(refSpace, strPlayerNickName, funCancel, b已营救坦克连);
					co_return 0;
				}
				CHECK_WP_CO_RET_0(responce.wpEntity);
				if (!wp虫巢右下.expired() && wp虫巢右下.lock() == responce.wpEntity.lock())//救出坦克连
				{
					b已营救坦克连 = true;
					std::weak_ptr<PlayerGateSession_Game> wpSession;
					while (true)
					{
						wpSession = GetPlayerGateSession(strPlayerNickName);
						if (!wpSession.expired() && !wpSession.lock()->m_wpSpace.expired() && wpSession.lock()->m_wpSpace.lock().get() == &refSpace)
							break;

						using namespace std;
						if (co_await CoTimer::Wait(5s, funCancel))//等玩家上线
							co_return true;
					}

					auto wp视口 = wpSession.lock()->m_wp视口;
					CHECK_WP_CO_RET_0(wp视口);
					auto& ref视口 = *wp视口.lock();

					for (int i = 0; i < 5; ++i)
					{
						refSpace.造活动单位(ref视口, strPlayerNickName, 三色坦克, { 05.f + i * 5 ,35.f });
						refSpace.造活动单位(ref视口, strPlayerNickName, 三色坦克, { 10.f + i * 5 ,40.f });
					}

					fun坦克手齐诺维说("谢谢你救了我们。");	_等玩家读完returnTrue;
					fun玩家说("你们怎么会在虫巢里？");	_等玩家读完returnTrue;
					fun坦克手齐诺维说("我也不知道，仿佛做了一场梦，什么都不记得了。");	_等玩家读完returnTrue;
					fun玩家说("现在我的部队会护送你们回基地。到时候再仔细回忆一下。");	_等玩家读完returnTrue;
					fun坦克手齐诺维说("上面还有两个虫巢，我要炸掉它们再回去。");	_等玩家读完returnTrue;
					fun玩家说("上面的虫巢有敌方的炮台把守，我们过不去。");	_等玩家读完returnTrue;
					fun坦克手齐诺维说("我的坦克专门克制炮台。<color=#a0ff50>坦克的射程略远于炮台</color>。一辆坦克就能全灭它们，何况我现在有十辆。");	_等玩家读完returnTrue;
					fun玩家说("好的，但是坦克要接受我的指挥，我的步兵会保护坦克。");	_等玩家读完returnTrue;
					fun坦克手齐诺维说("是！明白！指挥官！");	_等玩家读完returnTrue;
					PlayerComponent::剧情对话已看完(strPlayerNickName);
					PlayerGateSession_Game::Say任务提示(strPlayerNickName, "消灭上方两个敌方虫巢，控制好步兵保护您的坦克。<color=#a0ff50>让坦克在地方炮台的射程外攻击</color>。");
				}

			}
		}

		co_return 0;
	}

	CoTask<int> Co攻坚战_虫(Space& refSpace, FunCancel& funCancel, const std::string strPlayerNickName)
	{
		KeepCancel kc(funCancel);

		const auto fun房虫胡噜说 = [&strPlayerNickName, &refSpace](const std::string& str内容) {房虫胡噜说(refSpace, strPlayerNickName, str内容); };
		const auto fun玩家说 = [&strPlayerNickName, &refSpace](const std::string& str内容) {玩家_虫_说(refSpace, strPlayerNickName, str内容); };
		const auto fun绿色坦克洪隆说 = [&strPlayerNickName, &refSpace](const std::string& str内容) {绿色坦克洪隆说(refSpace, strPlayerNickName, str内容); };

		fun房虫胡噜说(strPlayerNickName + "，有一个好消息，我们的幼虫经过千万次的模仿，基本掌握了拟态复制人类三色坦克的方法。"); _等玩家读完returnTrue;
		fun玩家说("太好了……那么接下来肯定还有一个坏消息吧？");	_等玩家读完returnTrue;
		fun房虫胡噜说("是的，蜕变出的第一批<color = #a0ff50>绿色坦克</color>的虫巢已遭人类攻占。"); _等玩家读完returnTrue;
		fun玩家说("绿色坦克看来仍然打不过人类的部队啊！");	_等玩家读完returnTrue;
		fun房虫胡噜说("绿色坦克攻击距离远，但是面对贴近的人类步兵却处于劣势。此外绿色坦克发射出的光刺会被障碍物阻挡。现在人类的科学家正在研究我们的绿色坦克。"); _等玩家读完returnTrue;
		fun玩家说("我现在去救它们。");	_等玩家读完returnTrue;
		PlayerComponent::剧情对话已看完(strPlayerNickName);

		{
			{
				auto wpSession = GetPlayerGateSession(strPlayerNickName);
				CHECK_WP_CO_RET_0(wpSession);
				auto wp视口 = wpSession.lock()->m_wp视口;
				CHECK_WP_CO_RET_0(wp视口);
				auto& ref视口 = *wp视口.lock();

				for (int i = 0; i < 6; ++i)
				{
					const float z = 10.f + i * 5;
					refSpace.造活动单位(ref视口, strPlayerNickName, 单位类型::枪虫, { -40, z }, 2 == i);
					refSpace.造活动单位(ref视口, strPlayerNickName, 单位类型::近战虫, { -45, z });
				}
			}
			for (int i = 0; i < 3; ++i)
			{
				MonsterComponent::AddMonster(refSpace, 绿色坦克, { 40,-49.f + i * 5 }, 1);
				MonsterComponent::AddMonster(refSpace, 绿色坦克, { -40,-49.f + i * 5 }, 1);
			}
			for (int i = 0; i < 5; ++i)
			{
				//创建敌方建筑(refSpace, { 20,-49.f + i * 5 }, 炮台);
				创建敌方建筑(refSpace, { 25,-49.f + i * 5 }, 炮台);
				创建敌方建筑(refSpace, { 30,-49.f + i * 5 }, 炮台);

				//创建敌方建筑(refSpace, { -20,-49.f + i * 5 }, 炮台);
				创建敌方建筑(refSpace, { -25,-49.f + i * 5 }, 炮台);
				创建敌方建筑(refSpace, { -30,-49.f + i * 5 }, 炮台);
			}

			{
				//守卫右下角虫巢的怪
				MonsterComponent::AddMonster(refSpace, 枪兵怪, { 10,35.f }, 2);
				MonsterComponent::AddMonster(refSpace, 近战兵怪, { 10,49.f }, 2);
			}

			auto wp虫巢右下 = 创建敌方建筑(refSpace, { 10, 40.f }, 基地);
			创建敌方建筑(refSpace, { -45, -45.f }, 基地);
			创建敌方建筑(refSpace, { 45, -45.f }, 基地);

			PlayerGateSession_Game::Say任务提示(strPlayerNickName, "此局没有工虫。<color=#a0ff50>右下方的敌方基地防守薄弱</color>。");
			bool b已营救坦克连 = false;
			while (true)
			{
				auto [stop, responce] = co_await CoEvent<MyEvent::单位阵亡>::Wait(funCancel, __FUNCTION__, [&refSpace, wp虫巢右下](const MyEvent::单位阵亡& ref) {return &ref.wpEntity.lock()->m_refSpace == &refSpace; });
				if (stop)
					co_return 0;

				if (0 == refSpace.Get怪物单位数(基地))
				{
					co_await 攻坚战胜利_虫(refSpace, strPlayerNickName, funCancel);
					co_return 0;
				}

				if (0 == refSpace.Get玩家单位数(strPlayerNickName))
				{
					co_await 攻坚战失败_虫(refSpace, strPlayerNickName, funCancel, b已营救坦克连);
					co_return 0;
				}
				CHECK_WP_CO_RET_0(responce.wpEntity);
				if (!wp虫巢右下.expired() && wp虫巢右下.lock() == responce.wpEntity.lock())//救出坦克连
				{
					b已营救坦克连 = true;
					std::weak_ptr<PlayerGateSession_Game> wpSession;
					while (true)
					{
						wpSession = GetPlayerGateSession(strPlayerNickName);
						if (!wpSession.expired() && !wpSession.lock()->m_wpSpace.expired() && wpSession.lock()->m_wpSpace.lock().get() == &refSpace)
							break;

						using namespace std;
						if (co_await CoTimer::Wait(5s, funCancel))//等玩家上线
							co_return true;
					}

					auto wp视口 = wpSession.lock()->m_wp视口;
					CHECK_WP_CO_RET_0(wp视口);
					auto& ref视口 = *wp视口.lock();

					for (int i = 0; i < 5; ++i)
					{
						refSpace.造活动单位(ref视口, strPlayerNickName, 绿色坦克, { 05.f + i * 5 ,35.f });
						refSpace.造活动单位(ref视口, strPlayerNickName, 绿色坦克, { 10.f + i * 5 ,40.f });
					}

					fun绿色坦克洪隆说("谢谢你救了我们。");	_等玩家读完returnTrue;
					fun玩家说("你们怎么会在基地里？");	_等玩家读完returnTrue;
					fun绿色坦克洪隆说("我也不知道，仿佛做了一场梦，什么都不记得了。");	_等玩家读完returnTrue;
					fun玩家说("现在我的部队会护送你们回虫巢。到时候再仔细回忆一下。");	_等玩家读完returnTrue;
					fun绿色坦克洪隆说("上面还有两个基地，我要炸掉它们再回去。");	_等玩家读完returnTrue;
					fun玩家说("上面的基地有敌方的炮台把守，我们过不去。");	_等玩家读完returnTrue;
					fun绿色坦克洪隆说("我专门克制炮台。<color=#a0ff50>坦克的射程略远于炮台</color>。一辆坦克就能全灭它们，何况我们现在有十辆。");	_等玩家读完returnTrue;
					fun玩家说("好的，但是你们要接受我的指挥，我的部队会保护你们。");	_等玩家读完returnTrue;
					fun绿色坦克洪隆说("是！明白！指挥官！");	_等玩家读完returnTrue;
					PlayerComponent::剧情对话已看完(strPlayerNickName);
					PlayerGateSession_Game::Say任务提示(strPlayerNickName, "消灭上方两个敌方基地，控制好其它单位保护您的坦克。坦克的射程略远于炮台，<color=#a0ff50>不要让坦克进入敌方炮台的射程</color>。");
				}
			}
		}

		co_return 0;
	}
}