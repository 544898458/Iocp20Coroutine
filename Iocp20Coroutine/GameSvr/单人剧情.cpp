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
	static void 装甲指挥官海因茨说(Space& refSpace, const std::string& refStrNickName, const std::string& str内容)
	{
		PlayerComponent::剧情对话(refSpace, refStrNickName, "图片/大白胡子抽烟", "装甲指挥官：海因茨", "", "", "    " + str内容);
		PlayerComponent::播放声音(refStrNickName, "音效/BUTTON", "");
	}
	static void 玩家说(Space& refSpace, const std::string& strPlayerNickName, const std::string& str内容, const bool b显示退出场景按钮 = false)
	{
		PlayerComponent::剧情对话(refSpace, strPlayerNickName, "", "", "图片/指挥学员学员青灰色军服", "玩家：" + strPlayerNickName, "    " + str内容, b显示退出场景按钮);
		PlayerComponent::播放声音(strPlayerNickName, "音效/BUTTON", "");
	}

	static CoTask<bool> 等玩家读完(const std::string strPlayerNickName, FunCancel& funCancel)
	{
		const auto funSameSession = [&strPlayerNickName](const MyEvent::已阅读剧情对话& ref)
			{ return ref.wpPlayerGateSession.lock() == GetPlayerGateSession(strPlayerNickName).lock(); };

		auto [stop, event已阅读] = co_await CoEvent<MyEvent::已阅读剧情对话>::Wait(funCancel, funSameSession);
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

		const auto fun总教官陈近南说 = [&strPlayerNickName, &refSpace](const std::string& str内容) {总教官凌云说(refSpace, strPlayerNickName, str内容); };
		const auto fun玩家说 = [&strPlayerNickName, &refSpace](const std::string& str内容) {玩家说(refSpace, strPlayerNickName, str内容); };

		fun总教官陈近南说("即时战略游戏的操作并不复杂，老年间便有四句童谣：\n"
			"\t\t\t工程车，造基地；\n"
			"\t\t\t基地又产工程车。\n"
			"\t\t\t工程车，造兵厂，\n"
			"\t\t\t兵厂产兵欢乐多！"
		); _等玩家读完returnTrue;
		fun玩家说("听说工程车还可以造地堡和炮台，我也要试试。"); _等玩家读完returnTrue;
		fun总教官陈近南说("造完基地应该先安排工程车去采集晶体矿和燃气矿，这是一切生产建造的基础。此外建造民房可以提升活动单位上限。\n加油！"); _等玩家读完returnTrue;
		fun玩家说("我只想单手操作，拖动视口 和 选中多个单位 如何操作呢？"); _等玩家读完returnTrue;
		fun总教官陈近南说("\t\t拖动地面就可以移动视口，此外设置（齿轮图标）界面还有视口镜头投影切换、放大、缩小按钮。当然也支持双指缩放视口。\n"
			"\t\t先点击右边“框选”按钮，然后在屏幕中拖动，即可框选多个单位。在设置（齿轮图标）界面中可以切换菱形框选或方形框选。\n"
			"\t\t全程只要单手握持手机单指操作即可。\n"
		); _等玩家读完returnTrue;
		fun玩家说("只用一只手就能玩的RTS即时战略游戏，那岂不是跟刷短视频一样轻松？我一定要体验一下！"); _等玩家读完returnTrue;
		fun总教官陈近南说("走你!"); _等玩家读完returnTrue;
		PlayerComponent::剧情对话已看完(strPlayerNickName);
		using namespace std;
		PlayerGateSession_Game::Say任务提示(strPlayerNickName, "欢迎来到RTS即时战略游戏，现在您要接受基础的训练");

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
			//SpEntity sp工程车 = refSpace.造活动单位(wp视口.lock()->m_spPlayer, strPlayerNickName, { 5,10 }, 配置, 类型);
			refSpace.造活动单位(ref视口, strPlayerNickName, 单位类型::工程车, { 5, 10 }, true);
			资源Component::Add(refSpace, 晶体矿, { 20, 10 });
		}
		PlayerGateSession_Game::Say任务提示(strPlayerNickName, "请点击“工程车”选中，然后点击“建筑单位=>基地”按钮，再点击空白地面，10秒后就能造出一个基地");

		const auto funSameSpace = [&refSpace, &strPlayerNickName](const MyEvent::AddEntity& refAddEntity, 单位类型 类型)
			{ return MyEvent::SameSpace(refAddEntity.wpEntity, refSpace, strPlayerNickName) && EntitySystem::Is单位类型(refAddEntity.wpEntity, 类型); };

		Position pos基地;
		{
			const auto& [stop, addEvent] = co_await CoEvent<MyEvent::AddEntity>::Wait(funCancel, std::bind(funSameSpace, std::placeholders::_1, 基地));
			if (stop)
				co_return 0;

			pos基地 = addEvent.wpEntity.lock()->Pos();
		}


		if (co_await CoTimer::Wait(10s, funCancel))
			co_return 0;


		PlayerGateSession_Game::Say任务提示(strPlayerNickName, "请点击选中基地（圆环特效表示选中），然后点击“活动单位/工程车”按钮，5秒后会在基地旁造出一个工程车");

		if (std::get<0>(co_await CoEvent<MyEvent::AddEntity>::Wait(funCancel, std::bind(funSameSpace, std::placeholders::_1, 工程车))))
			co_return 0;

		PlayerGateSession_Game::Say任务提示(strPlayerNickName, "现在您有工程车了，点击选中您的工程车，再点击空旷地面，命令它走向目标点");

		if (std::get<0>(co_await CoEvent<MyEvent::MoveEntity>::Wait(funCancel, [&refSpace, &strPlayerNickName](const MyEvent::MoveEntity& ref)
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

		PlayerGateSession_Game::Say任务提示(strPlayerNickName, "很好！现在给您刷了一个晶体矿，请点击晶体矿，让工程车在晶体矿和基地之间搬运晶体矿");
		资源Component::Add(refSpace, 晶体矿, { pos基地.x,pos基地.z - 20 });
		资源Component::Add(refSpace, 燃气矿, { pos基地.x + 15,pos基地.z });

		if (std::get<0>(co_await CoEvent<MyEvent::开始采集晶体矿>::Wait(funCancel)))
			co_return 0;

		PlayerGateSession_Game::Say任务提示(strPlayerNickName, "很好！您的工程车正在采集晶体矿，请等他把晶体矿运回基地。现在，请选中另一辆工程车去采集燃气矿");
		if (std::get<0>(co_await CoEvent<MyEvent::晶体矿已运回基地>::Wait(funCancel)))
			co_return 0;

		PlayerGateSession_Game::Say任务提示(strPlayerNickName, "您的工程车已把第一车晶体矿运到基地，请查看左上角晶体矿数量变化");

		if (co_await CoTimer::Wait(5s, funCancel))
			co_return 0;

		PlayerGateSession_Game::Say任务提示(strPlayerNickName, "等您存够20晶体矿后，请选中一辆工程车，然后点击“建筑单位/民房”按钮，再点击一次空旷地面");
		if (std::get<0>(co_await CoEvent<MyEvent::AddEntity>::Wait(funCancel, std::bind(funSameSpace, std::placeholders::_1, 民房))))
			co_return 0;

		PlayerGateSession_Game::Say任务提示(strPlayerNickName, "很好，民房可以提升您的活动单位数量上限");

		if (co_await CoTimer::Wait(3s, funCancel))
			co_return 0;

		PlayerGateSession_Game::Say任务提示(strPlayerNickName, "等您存够30晶体矿后，请选中一辆工程车，然后点击“建筑单位/兵厂”按钮，再点击一次空旷地面，就能造出一个兵厂");
		if (std::get<0>(co_await CoEvent<MyEvent::AddEntity>::Wait(funCancel, std::bind(funSameSpace, std::placeholders::_1, 兵厂))))
			co_return 0;

		if (co_await CoTimer::Wait(10s, funCancel))
			co_return 0;

		PlayerGateSession_Game::Say任务提示(strPlayerNickName, "请点击选中兵厂（圆环特效表示选中），然后点击“活动单位/兵”按钮，10秒后会在兵厂旁造出一个兵");

		if (std::get<0>(co_await CoEvent<MyEvent::AddEntity>::Wait(funCancel, std::bind(funSameSpace, std::placeholders::_1, 兵))))
			co_return 0;

		PlayerGateSession_Game::Say任务提示(strPlayerNickName, "点击选中您的兵，再点击地面，可以指挥他走向目标处");

		if (std::get<0>(co_await CoEvent<MyEvent::MoveEntity>::Wait(funCancel, [&refSpace, &strPlayerNickName](const MyEvent::MoveEntity& ref) {
			return //&ref.wpEntity.lock()->m_refSpace == &refSpace; 
				MyEvent::SameSpace(ref.wpEntity, refSpace, strPlayerNickName) && EntitySystem::Is单位类型(ref.wpEntity, 兵);
			})))
			co_return 0;

			PlayerGateSession_Game::Say任务提示(strPlayerNickName, "现在已在左边给您刷了一个怪，控制兵走到怪附近，兵会自动打怪。您可以点右下角“取消选中”然后拖动地面看看怪在哪里");
			MonsterComponent::AddMonster(refSpace, 兵, { -30,20 });

			if (std::get<0>(co_await CoEvent<MyEvent::单位阵亡>::Wait(funCancel, [&refSpace](const MyEvent::单位阵亡& ref) {return &ref.wpEntity.lock()->m_refSpace == &refSpace; })))
				co_return 0;

			if (0 < refSpace.Get怪物单位数())
			{
				PlayerGateSession_Game::Say任务提示(strPlayerNickName, "您的兵阵亡了。可以多造点兵去围攻敌人");

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
			}


			PlayerGateSession_Game::Say任务提示(strPlayerNickName, "恭喜您消灭了敌人！现在左边给您刷了更多的怪。您可以造“活动单位/地堡”,让兵进入地堡中，立足防守，再伺机进攻");
			MonsterComponent::AddMonster(refSpace, 近战兵, { -30.0 }, 5);

			if (std::get<0>(co_await CoEvent<MyEvent::单位阵亡>::Wait(funCancel, [&refSpace, &strPlayerNickName](const MyEvent::单位阵亡& ref)
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

				fun总教官陈近南说(strPlayerNickName + "，你的悟性很高，10分钟就完成了其他学员一个月才能完成的训练，为师甚是欣慰！"); _等玩家读完returnTrue;
				fun玩家说("……我这都是些什么同学啊，这么简单的操作还要学一个月吗？"); _等玩家读完returnTrue;
				fun总教官陈近南说("唉，谁说不是呢！如今世风不古。遥想20年前，比你悟性更高的指挥天才比比皆是，大家经常自发研究切磋指挥技艺，更有无数学员的水平远超教官。实在令人怀念……"); _等玩家读完returnTrue;
				fun玩家说("这些前辈果真如此厉害吗？学生倒是想领教领教！"); _等玩家读完returnTrue;
				fun总教官陈近南说("在“多人联机地图”和“其他人的多人战局列表”可能会遇到他们，不要轻敌。要善用战斗单位的克制关系：\n"
					"\t\t近战兵 克制 坦克\n"
					"\t\t坦克 克制 地堡和炮台\n"
					"\t\t地堡和炮台 克制 近战兵\n"
				); _等玩家读完returnTrue;
				fun玩家说("这些克制关系是怎么产生的呢？"); _等玩家读完returnTrue;
				fun总教官陈近南说("坦克价格昂贵，攻击前摇最久，移动速度最慢；前摇开始后炸点无法改变，敌方单位很容易躲开炸点，此外坦克炸点溅射会伤害附近的己方单位。\n"
					"\t\t近战兵移动速度快，攻击速度快，价格便宜，很容易躲开坦克炸点，也很容易在坦克攻击前摇结束之前将坦克打掉。"); _等玩家读完returnTrue;
				fun总教官陈近南说("坦克，攻击距离最远，可以在炮台和地堡内单位的射程外攻击，是拆除建筑的利器。"); _等玩家读完returnTrue;
				fun总教官陈近南说("炮台价格便宜，攻击速度快，攻击距离仅次于坦克，集中放置后可对快速移动的敌方小兵群体造成有效伤害。"); _等玩家读完returnTrue;
				fun玩家说("我会在实战中体会摸索。"); _等玩家读完returnTrue;
				fun总教官陈近南说("一个人的时候，可以试试“防守战”，有助于体会炮台、坦克、地堡的威力。"); _等玩家读完returnTrue;
				玩家说(refSpace, strPlayerNickName, "好的。", true);//	_等玩家读完;
			}
			else
			{
				PlayerComponent::播放声音(strPlayerNickName, "音效/YouLoss", "训练失败。");

				fun总教官陈近南说("这只是一次训练，不必过于在意。失败是成功之母，待情绪平复可以再试一次。"); _等玩家读完returnTrue;
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
		const auto fun总教官陈近南说 = [&strPlayerNickName, &refSpace](const std::string& str内容) {总教官凌云说(refSpace, strPlayerNickName, str内容); };
		const auto fun玩家说 = [&strPlayerNickName, &refSpace](const std::string& str内容) {玩家说(refSpace, strPlayerNickName, str内容); };

		if (0 == refSpace.Get怪物单位数())
		{
			PlayerGateSession_Game::播放声音(strPlayerNickName, "音效/YouWin", "您守住了！您真是指挥天才！");

			fun总教官陈近南说(strPlayerNickName + "，我果然没有看错你，你是一个指挥天才！"); _等玩家读完returnTrue;
			fun玩家说("承蒙教官谬赞。学生谨遵教诲，自不敢有一丝懈怠，定当不负所望，继续精进。");	_等玩家读完returnTrue;
			PlayerComponent::剧情对话(refSpace, strPlayerNickName, "图片/陈近南", "总教官：陈近南", "", "", "    走你!", true);
			co_return true;
		}
		if (0 == refSpace.Get玩家单位数(strPlayerNickName))
		{
			PlayerGateSession_Game::播放声音(strPlayerNickName, "音效/YouLose", "胜败乃兵家常事，请点击右上角“退出场景”离开，然后再次点击“防守战”，就可以重新来过。");
			fun总教官陈近南说("胜败乃兵家常事，我仍然看好你的潜力！"); _等玩家读完returnTrue;
			玩家说(refSpace, strPlayerNickName, "学生此次功败垂成，定会吸取教训，总结经验，再次努力，定不辜负教官的栽培与期望。", true);//	_等玩家读完;
			co_return true;
		}
		co_return false;
	}
	CoTask<int> Co防守战(Space& refSpace, FunCancel& funCancel, const std::string strPlayerNickName)
	{
		KeepCancel kc(funCancel);
		using namespace std;

		const auto fun总教官陈近南说 = [&strPlayerNickName, &refSpace](const std::string& str内容) {总教官凌云说(refSpace, strPlayerNickName, str内容); };
		const auto fun玩家说 = [&strPlayerNickName, &refSpace](const std::string& str内容) {玩家说(refSpace, strPlayerNickName, str内容); };

		fun总教官陈近南说("情报显示：将有大量敌方单位进攻我方基地，请做好准备。");	_等玩家读完returnTrue;
		fun玩家说("可是我仅受过简单的指挥训练。");							_等玩家读完returnTrue;
		fun总教官陈近南说("我们会提供足够的初始晶体矿和燃气矿，你不用再从零开始采集。");	_等玩家读完returnTrue;
		fun玩家说("我还是没有把握。");										_等玩家读完returnTrue;
		fun总教官陈近南说("我会传授你指挥实战速成口诀，三分钟即可成为实战高手。");	_等玩家读完returnTrue;
		fun玩家说("真有这种口诀的话就应该在训练战时告诉我啊！");			_等玩家读完returnTrue;
		fun总教官陈近南说("实战口诀只有在九死一生的实战中才有用！");		_等玩家读完returnTrue;
		fun玩家说("九死一生？形势严峻啊，我会全力以赴的!");					_等玩家读完returnTrue;
		fun总教官陈近南说("坦克可以超远距离造成超高伤害，而且是范围伤害，但是坦克锁定炮弹落点后有超长前摇时长，前摇结束后，敌方目标可能已经远离炮弹落点。炮弹落点附近的我方单位也会受到伤害。"); _等玩家读完returnTrue;
		fun玩家说("有点鸡肋的感觉，不是红警中的高速坦克，与星际中的攻城坦克也有些不同。");	_等玩家读完returnTrue;
		fun总教官陈近南说("…不知道你在说什么…有一种叫“炮台”的建筑单位，攻击距离很远，仅次于坦克；前摇时长很短，可以快速攻击；缺陷是伤害较低；不过多造点集中防守某一块区域还是很实用的。"); _等玩家读完returnTrue;
		fun玩家说("看来只要有坦克和炮台就能守住对吗？");					_等玩家读完returnTrue;
		fun总教官陈近南说("是的，这是其他指挥官总结的经验。大概10个炮台和5辆坦克沿墙边布置就能守住。当然还有其他多种可行的防守方法，要靠你自行摸索。"); _等玩家读完returnTrue;
		fun玩家说("OK，也不是很复杂。我有信心完成任务！");					_等玩家读完returnTrue;
		fun总教官陈近南说("走你！");										_等玩家读完returnTrue;

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
			refSpace.造活动单位(*wp视口.lock(), strPlayerNickName, 单位类型::工程车, { -10, 10 });


			资源Component::Add(refSpace, 晶体矿, { -20, 35 });
			资源Component::Add(refSpace, 晶体矿, { -26, 20 });
			资源Component::Add(refSpace, 燃气矿, { -29, 35 });
			资源Component::Add(refSpace, 燃气矿, { -20, 20 });

		}

		PlayerGateSession_Game::Say任务提示(strPlayerNickName, "5秒后将出现第1波怪。您可以沿右边墙边造炮台、布置坦克、造地堡、让兵进驻地堡。");
		if (co_await CoTimer::Wait(5s, funCancel))
			co_return 0;

		PlayerGateSession_Game::Say任务提示(strPlayerNickName, "不用采矿。沿右边迷宫出口墙边布置一排炮台，再放置坦克加强防御。");

		for (int i = 1; i < 20; ++i)
		{
			if (1 < i && co_await Is战斗结束(refSpace, strPlayerNickName, funCancel))
				co_return 0;

			PlayerGateSession_Game::Say系统(strPlayerNickName, std::format("第{0}波敌人正向您走来", i));
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
		refEntity.m_spBuilding->直接造好();
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
			fun装甲指挥官海因茨说("可以试试操作慢一点，先进攻右下角的虫巢。"); _等玩家读完returnTrue;
			玩家说(refSpace, strPlayerNickName, "只能下次再试试了。", true);
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

		fun装甲指挥官海因茨说(strPlayerNickName + "，我的一个坦克连在调查无人区虫巢时失联，请前往营救。"); _等玩家读完returnTrue;
		fun玩家说("坦克没有步兵保护吗？");	_等玩家读完returnTrue;
		fun装甲指挥官海因茨说("是的，步兵刚刚到位，两个班的兵力，现在全部交给你指挥。"); _等玩家读完returnTrue;
		fun玩家说("虫巢是什么？无人区为什么会有虫巢呢？");	_等玩家读完returnTrue;
		fun科学家玛丽亚说("虫巢是什么尚不清楚，好像是凭空出现的。无人侦察机拍摄的视频表明虫巢周围会形成无人区，人的去向不明。");	_等玩家读完returnTrue;
		fun玩家说("我想起一个古老的传说……");	_等玩家读完returnTrue;
		fun装甲指挥官海因茨说("现在不是讲故事的时候，坦克连的伴随无人机也失联了。我们再次派侦察机到目标上空已找不到坦克连的踪迹。"); _等玩家读完returnTrue;
		fun玩家说("一个坦克连应该有10辆坦克吧？这么多钢铁疙瘩能凭空消失吗？");	_等玩家读完returnTrue;
		fun科学家玛丽亚说("无人区本来是一个生活区，居住着很多人，但是出现虫巢后，居民都消失了。坦克连正是去调查居民失踪的事。");	_等玩家读完returnTrue;
		fun玩家说("居民失踪就派坦克连去调查，坦克连消失就派我的部队去营救。要是我和我的部队也消失了，还会有人去救我们吗？");	_等玩家读完returnTrue;
		fun装甲指挥官海因茨说("现在人手紧缺，我不能做任何行动上的承诺。但我个人会竭尽全力，不放弃任何一个人，就像我不会放弃坦克连一样。"); _等玩家读完returnTrue;
		fun玩家说("我也会尽力而为的。");	_等玩家读完returnTrue;
		fun装甲指挥官海因茨说("要多用脑子，要多想。记住兵种克制关系：\n小兵 克制 坦克\n坦克 克制 炮台\n炮台 克制 小兵"); _等玩家读完returnTrue;
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
					refSpace.造活动单位(ref视口, strPlayerNickName, 单位类型::兵, { -40, z });
					refSpace.造活动单位(ref视口, strPlayerNickName, 单位类型::近战兵, { -45, z });
				}
			}
			for (int i = 0; i < 6; ++i)
			{
				MonsterComponent::AddMonster(refSpace, 三色坦克, { 40,-49.f + i * 5 }, 1);
				MonsterComponent::AddMonster(refSpace, 三色坦克, { -40,-49.f + i * 5 }, 1);
			}
			for (int i = 0; i < 6; ++i)
			{
				创建敌方建筑(refSpace, { 25,-49.f + i * 5 }, 炮台);
				创建敌方建筑(refSpace, { 30,-49.f + i * 5 }, 炮台);
				创建敌方建筑(refSpace, { 35,-49.f + i * 5 }, 炮台);

				创建敌方建筑(refSpace, { -25,-49.f + i * 5 }, 炮台);
				创建敌方建筑(refSpace, { -30,-49.f + i * 5 }, 炮台);
				创建敌方建筑(refSpace, { -35,-49.f + i * 5 }, 炮台);
			}

			{
				//守卫右下角虫巢的怪
				MonsterComponent::AddMonster(refSpace, 兵, { 10,35.f }, 2);
				MonsterComponent::AddMonster(refSpace, 近战兵, { 10,49.f }, 2);
			}

			auto wp虫巢右下 = 创建敌方建筑(refSpace, { 10, 40.f }, 虫巢);
			创建敌方建筑(refSpace, { -45, -45.f }, 虫巢);
			创建敌方建筑(refSpace, { 45, -45.f }, 虫巢);

			PlayerGateSession_Game::Say任务提示(strPlayerNickName, "此局没有工程车。右下方的敌方虫巢防守薄弱。");
			bool b已营救坦克连 = false;
			while (true)
			{
				auto [stop, responce] = co_await CoEvent<MyEvent::单位阵亡>::Wait(funCancel, [&refSpace, wp虫巢右下](const MyEvent::单位阵亡& ref) {return &ref.wpEntity.lock()->m_refSpace == &refSpace; });
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
					fun坦克手齐诺维说("我的坦克专门克制炮台。一辆坦克就能全灭它们，何况我现在有十辆。");	_等玩家读完returnTrue;
					fun玩家说("好的，但是坦克要接受我的指挥，我的步兵会保护坦克。");	_等玩家读完returnTrue;
					fun坦克手齐诺维说("是！明白！指挥官！");	_等玩家读完returnTrue;
					PlayerComponent::剧情对话已看完(strPlayerNickName);
					PlayerGateSession_Game::Say任务提示(strPlayerNickName, "消灭上方两个敌方虫巢，控制好步兵保护您的坦克");
				}

			}
		}

		co_return 0;
	}
}