#include "pch.h"
#include "混战.h"
#include "EntitySystem.h"
#include "Space.h"
#include "单位组件/PlayerComponent.h"
#include "../CoRoutine/CoTimer.h"
#include "PlayerGateSession_Game.h"

namespace 混战
{
	CoTaskBool Co进多人联机混战(Space& refSpace, Entity& ref视口, const uint16_t u16半径, const bool b已看完激励视频广告, FunCancel安全& refFunCancel进地图)
	{
		const std::string strPlayerNickName = EntitySystem::GetNickName(ref视口);
		{
			const Position pos出生 = 在方块里随机一个点(u16半径);
			// auto pos出生 = Position(std::rand() % 100 - 50.f, 0, std::rand() % 100 - 50.f);
			const uint16_t 间距 = 5;
			refSpace.造活动单位(ref视口, strPlayerNickName, 单位类型::工程车, pos出生, true);
			refSpace.造活动单位(ref视口, strPlayerNickName, 单位类型::工虫, { pos出生.x + 间距, 0, pos出生.z + 间距 });
			if (b已看完激励视频广告)
			{
				refSpace.造活动单位(ref视口, strPlayerNickName, 单位类型::枪兵, { pos出生.x, 0, pos出生.z + 间距 });
				refSpace.造活动单位(ref视口, strPlayerNickName, 单位类型::三色坦克, { pos出生.x + 间距, 0, pos出生.z });
				refSpace.造活动单位(ref视口, strPlayerNickName, 单位类型::近战兵, { pos出生.x - 间距, 0, pos出生.z });
				refSpace.造活动单位(ref视口, strPlayerNickName, 单位类型::飞机, { pos出生.x - 间距, 0, pos出生.z - 间距 });
				refSpace.造活动单位(ref视口, strPlayerNickName, 单位类型::幼虫, { pos出生.x + 间距, 0, pos出生.z - 间距 });
				refSpace.造活动单位(ref视口, strPlayerNickName, 单位类型::绿色坦克, { pos出生.x - 间距, 0, pos出生.z + 间距 });
				refSpace.造活动单位(ref视口, strPlayerNickName, 单位类型::近战虫, { pos出生.x, 0, pos出生.z - 间距 });
				refSpace.造活动单位(ref视口, strPlayerNickName, 单位类型::枪虫, { pos出生.x, 0, pos出生.z - 间距 * 2 });
				refSpace.造活动单位(ref视口, strPlayerNickName, 单位类型::房虫, { pos出生.x - 间距, 0, pos出生.z - 间距 * 2 });
				refSpace.造活动单位(ref视口, strPlayerNickName, 单位类型::飞虫, { pos出生.x - 间距 * 2, 0, pos出生.z - 间距 * 2 });
				refSpace.造活动单位(ref视口, strPlayerNickName, 单位类型::医疗兵, { pos出生.x + 间距 * 2, 0, pos出生.z + 间距 * 2 });
				refSpace.造活动单位(ref视口, strPlayerNickName, 单位类型::防空兵, { pos出生.x, 0, pos出生.z + 间距 * 2 });
			}
			// auto [stop, msgResponce] = co_await AiCo::ChangeMoney(*this, 0, true, refFunCancel进地图);
			// if (stop)
			//	co_return true;

			auto& refSpacePlayer = refSpace.GetSpacePlayer(ref视口);
			// if (refSpacePlayer.m_u32晶体矿 < u16初始晶体矿)
			{
				const uint16_t u16初始晶体矿 = b已看完激励视频广告 ? 1000 : 80;
				refSpacePlayer.m_u32晶体矿 += u16初始晶体矿;
				refSpacePlayer.m_u32燃气矿 += u16初始晶体矿;
				PlayerComponent::Send资源(ref视口);
			}
			// if (msgResponce.finalMoney < u16初始晶体矿)
			//{
			// if (std::get<0>(co_await AiCo::ChangeMoney(*this, u16初始晶体矿, true, refFunCancel进地图)))
			//	co_return true;
			//}
		}

		using namespace std;
		const auto seconds消息间隔 = 10s;
		PlayerGateSession_Game::Say系统(strPlayerNickName, "这是每个玩家都可以自由共同进入的场景，分布有一些资源和少量的怪，资源的再生速度很慢");
		if (co_await CoTimer::Wait(seconds消息间隔, refFunCancel进地图))
			co_return false;
		PlayerGateSession_Game::Say系统(strPlayerNickName, "您开局只有一辆工程车，工程车可以建造建筑，建筑中可以产出活动单位");
		if (co_await CoTimer::Wait(seconds消息间隔, refFunCancel进地图))
			co_return false;
		PlayerGateSession_Game::Say系统(strPlayerNickName, "一旦您退出此场景或断线后，您在此场景中的所有单位可能会因为缺少指挥而遭到攻击");
		if (co_await CoTimer::Wait(seconds消息间隔, refFunCancel进地图))
			co_return false;
		PlayerGateSession_Game::Say系统(strPlayerNickName, "其他玩家是您的敌人，他们可能会向您进攻！");
		if (co_await CoTimer::Wait(seconds消息间隔, refFunCancel进地图))
			co_return false;
		PlayerGateSession_Game::Say系统(strPlayerNickName, "如果局势对您不利，您可以退出此场景再次进入，就有机会东山再起！");
		if (co_await CoTimer::Wait(seconds消息间隔, refFunCancel进地图))
			co_return false;
		PlayerGateSession_Game::Say系统(strPlayerNickName, "请施展您的指挥艺术吧，加油！");

		co_return false;
	}
}