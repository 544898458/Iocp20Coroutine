#include "pch.h"
#include "多人战局.h"
#include "Space.h"
#include "PlayerGateSession_Game.h"
#include "../CoRoutine/CoEvent.h"
#include "MyEvent.h"
#include "EntitySystem.h"
#include "单位组件/资源Component.h"

std::weak_ptr<PlayerGateSession_Game> GetPlayerGateSession(const std::string &refStrNickName);

void 玩家带入单位进入四方对战(Space &refSpace, const Position &refPos, PlayerGateSession_Game &refGateSession)
{
	CHECK_WP_RET_VOID(refGateSession.m_wp视口);
	refSpace.造活动单位(*refGateSession.m_wp视口.lock(), refGateSession.NickName(), 单位类型::工程车, refPos, true);
	refSpace.造活动单位(*refGateSession.m_wp视口.lock(), refGateSession.NickName(), 单位类型::工虫, {refPos.x, 0, refPos.z + 5});

	const uint16_t 内圈半径 = 15;
	const uint16_t 外圈半径 = 18;
	资源Component::Add(refSpace, 晶体矿, {refPos.x, 0, refPos.z - 内圈半径});
	资源Component::Add(refSpace, 晶体矿, {refPos.x, 0, refPos.z - 外圈半径});
	资源Component::Add(refSpace, 晶体矿, {refPos.x, 0, refPos.z + 内圈半径});
	资源Component::Add(refSpace, 晶体矿, {refPos.x, 0, refPos.z + 外圈半径});
	资源Component::Add(refSpace, 晶体矿, {refPos.x - 内圈半径, 0, refPos.z});
	资源Component::Add(refSpace, 晶体矿, {refPos.x - 外圈半径, 0, refPos.z});
	资源Component::Add(refSpace, 燃气矿, {refPos.x + 内圈半径, 0, refPos.z});
	资源Component::Add(refSpace, 燃气矿, {refPos.x + 外圈半径, 0, refPos.z});
}

struct 方位玩家
{
	Position pos出生点;
	std::string strNickName;
};

static CoTask<int> Co多人对战(Space &refSpace, FunCancel &funCancel, const std::string strPlayerNickName, std::vector<方位玩家> &vec方位玩家)
{
	// 房主进第一个点
	vec方位玩家[0].strNickName = strPlayerNickName;
	auto spOwnerPlayerSession = GetPlayerGateSession(strPlayerNickName);
	CHECK_WP_CO_RET_0(spOwnerPlayerSession);
	玩家带入单位进入四方对战(refSpace, vec方位玩家[0].pos出生点, *spOwnerPlayerSession.lock());

	for (int i = 1; i < vec方位玩家.size(); ++i)
	{
		const auto &[stop, event玩家进入Space] = co_await CoEvent<MyEvent::玩家进入Space>::Wait(funCancel, __FUNCTION__,
																								[&refSpace](auto &refPlayer)
																								{ return &*refPlayer.wpSpace.lock() == &refSpace; });
		if (stop)
		{
			LOG(INFO) << "Co四方对战,中断退出";
			co_return 0;
		}

		CHECK_WP_CO_RET_0(event玩家进入Space.wpPlayerGateSession);
		CHECK_WP_CO_RET_0(event玩家进入Space.wpSpace);
		CHECK_WP_CO_RET_0(event玩家进入Space.wp视口);

		{
			const auto iterEnd = vec方位玩家.end();
			if (iterEnd != std::find_if(vec方位玩家.begin(), iterEnd,
										[&event玩家进入Space](const 方位玩家 &ref方位玩家)
										{ return ref方位玩家.strNickName == event玩家进入Space.wpPlayerGateSession.lock()->NickName(); }))
			{
				LOG(INFO) << "重复进入：" << event玩家进入Space.wpPlayerGateSession.lock()->NickName();
				continue;
			}
		}
		// event玩家进入Space.wpPlayerGateSession.lock()->EnterSpace(wpSpace);
		vec方位玩家[i].strNickName = event玩家进入Space.wpPlayerGateSession.lock()->NickName();
		玩家带入单位进入四方对战(refSpace, vec方位玩家[i].pos出生点, *event玩家进入Space.wpPlayerGateSession.lock());
		++i;
	}
	co_return 0;
}

CoTask<int> 多人战局::Co四方对战(Space &refSpace, FunCancel &funCancel, const std::string strPlayerNickName)
{
	const uint16_t u16距离中心点 = 50;
	std::vector<方位玩家> vec方位玩家 =
		{
			{{u16距离中心点, 0, 0}, ""},
			{{-u16距离中心点, 0, 0}, ""},
			{{0, 0, u16距离中心点}, ""},
			{{0, 0, -u16距离中心点}, ""},
		};
	co_return co_await Co多人对战(refSpace, funCancel, strPlayerNickName, vec方位玩家);
}

void 造资源(Space &refSpace, const Position &refPos)
{
	const uint16_t u16资源距离 = 2;
	资源Component::Add(refSpace, 晶体矿, refPos + Position{0, 0, u16资源距离});
	资源Component::Add(refSpace, 晶体矿, refPos + Position{u16资源距离, 0, 0});
	资源Component::Add(refSpace, 晶体矿, refPos + Position{-u16资源距离, 0, 0});
	资源Component::Add(refSpace, 燃气矿, refPos + Position{0, 0, -u16资源距离});

}

CoTask<int> 多人战局::Co一打一(Space &refSpace, FunCancel &funCancel, const std::string strPlayerNickName)
{
	造资源(refSpace, {50, 0, -35});
	造资源(refSpace, {42, 0, -45});
	造资源(refSpace, {22, 0, 20});
	造资源(refSpace, {-8, 0, 57});
	造资源(refSpace, {8, 0, 57});
	造资源(refSpace, {-40, 0, 15});
	std::vector<方位玩家> vec方位玩家 =
		{
			{{-16, 0, -110}, ""},
			{{-90, 0, -70}, ""},
		};
	co_return co_await Co多人对战(refSpace, funCancel, strPlayerNickName, vec方位玩家);
}
