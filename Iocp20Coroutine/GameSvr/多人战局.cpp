#include "pch.h"
#include "多人战局.h"
#include "Space.h"
#include "PlayerGateSession_Game.h"
#include "../CoRoutine/CoEvent.h"
#include "MyEvent.h"
#include "EntitySystem.h"
#include "单位组件/资源Component.h"

std::weak_ptr<PlayerGateSession_Game> GetPlayerGateSession(const std::string& refStrNickName);

void 玩家带入单位进入四方对战(Space& refSpace, const Position &refPos, PlayerGateSession_Game& refGateSession)
{
	{
		const 单位类型 类型(单位类型::工程车);
		单位::活动单位配置 配置;
		单位::Find活动单位配置(类型, 配置);
		CHECK_WP_RET_VOID(refGateSession.m_wp视口);
		SpEntity sp工程车 = refSpace.造活动单位(refGateSession.m_wp视口.lock()->m_spPlayer, refGateSession.NickName(), refPos, 配置, 类型);
		refGateSession.Send设置视口(*sp工程车);
	}

	资源Component::Add(refSpace, 晶体矿, { refPos.x,		refPos.z - 20 });
	资源Component::Add(refSpace, 晶体矿, { refPos.x,		refPos.z + 20 });
	资源Component::Add(refSpace, 晶体矿, { refPos.x - 20,	refPos.z	});
	资源Component::Add(refSpace, 燃气矿, { refPos.x + 20,	refPos.z	});

}

CoTask<int> 多人战局::Co四方对战(Space& refSpace, FunCancel& funCancel, const std::string strPlayerNickName)
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
	arr方位玩家[0].strNickName = strPlayerNickName;
	auto spOwnerPlayerSession = GetPlayerGateSession(strPlayerNickName);
	CHECK_WP_CO_RET_0(spOwnerPlayerSession);
	玩家带入单位进入四方对战(refSpace, arr方位玩家[0].pos出生点, *spOwnerPlayerSession.lock());

	for (int i = 1; i < _countof(arr方位玩家); )
	{
		const auto& [stop, event玩家进入Space] = co_await CoEvent<MyEvent::玩家进入Space>::Wait(funCancel, 
			[&refSpace](auto& refPlayer) { return &*refPlayer.wpSpace.lock() == &refSpace; });
		if (stop)
		{
			LOG(INFO) << "Co四方对战,中断退出";
			co_return 0;
		}

		CHECK_WP_CO_RET_0(event玩家进入Space.wpPlayerGateSession);
		CHECK_WP_CO_RET_0(event玩家进入Space.wpSpace);
		CHECK_WP_CO_RET_0(event玩家进入Space.wp视口);

		{
			auto iterEnd = arr方位玩家 + _countof(arr方位玩家);
			if (iterEnd != std::find_if(arr方位玩家, iterEnd, 
				[&event玩家进入Space](const 方位玩家& ref方位玩家) 
				{return ref方位玩家.strNickName == event玩家进入Space.wpPlayerGateSession.lock()->NickName(); }))
			{
				LOG(INFO) << "重复进入：" << event玩家进入Space.wpPlayerGateSession.lock()->NickName();
				continue;
			}
		}
		//event玩家进入Space.wpPlayerGateSession.lock()->EnterSpace(wpSpace);
		arr方位玩家[i].strNickName = event玩家进入Space.wpPlayerGateSession.lock()->NickName();
		玩家带入单位进入四方对战(refSpace, arr方位玩家[i].pos出生点, *event玩家进入Space.wpPlayerGateSession.lock());
		++i;
	}
	co_return 0;
}
