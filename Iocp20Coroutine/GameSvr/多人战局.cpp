#include "pch.h"
#include "多人战局.h"
#include "Space.h"
#include "PlayerGateSession_Game.h"
#include "../CoRoutine/CoEvent.h"
#include "MyEvent.h"
#include "EntitySystem.h"

void 玩家带入单位进入四方对战(Space& refSpace, Entity& ref视口, const Position &refPos )
{
	const 单位类型 类型(单位类型::工程车);
	单位::活动单位配置 配置;
	单位::Find活动单位配置(类型, 配置);
	SpEntity sp工程车 = refSpace.造活动单位(ref视口.m_spPlayer, EntitySystem::GetNickName(ref视口), refPos, 配置, 类型);
}

CoTask<int> 多人战局::Co四方对战(Space& refSpace, Entity& ref视口, FunCancel& funCancel, PlayerGateSession_Game& refGateSession)
{
	struct 方位玩家
	{
		Position pos出生点;
		std::string strNickName;
	};

	const uint16_t u16距离中心点 = 50;
	方位玩家 arr方位玩家[] = 
	{
		{{u16距离中心点	,		0			},""},
		{{-u16距离中心点,		0			},""},
		{{			0	,	u16距离中心点	},""},
		{{			0	,	-u16距离中心点	},""},
	};

	//房主进第一个点
	arr方位玩家[0].strNickName = refGateSession.NickName();
	auto wpSpace = refGateSession.m_wpSpace;
	玩家带入单位进入四方对战(refSpace, ref视口, arr方位玩家[0].pos出生点);

	for (int i = 1; i < _countof(arr方位玩家); ++i)
	{
		const auto& [stop, event玩家进入Space] = co_await CoEvent<MyEvent::玩家进入Space>::Wait(funCancel, [&refSpace]
		(auto& refPlayer) { return &*refPlayer.wpSpace.lock() == &refSpace; });
		if (stop)
			co_return 0;

		CHECK_WP_CO_RET_0(wpSpace);
		CHECK_WP_CO_RET_0(event玩家进入Space.wpPlayerGateSession);
		CHECK_WP_CO_RET_0(event玩家进入Space.wpSpace);
		CHECK_WP_CO_RET_0(event玩家进入Space.wp视口);
		event玩家进入Space.wpPlayerGateSession.lock()->EnterSpace(wpSpace);
		arr方位玩家[i].strNickName = event玩家进入Space.wpPlayerGateSession.lock()->NickName();
		玩家带入单位进入四方对战(refSpace, *event玩家进入Space.wp视口.lock(), arr方位玩家[i].pos出生点);
	}
	co_return 0;
}
