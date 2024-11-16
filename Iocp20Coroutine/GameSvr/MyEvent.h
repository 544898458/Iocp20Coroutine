#pragma once
#include "SpEntity.h"
class Space;
class PlayerGateSession_Game;
namespace MyEvent
{
	struct AddEntity
	{
		WpEntity wpEntity;
	};
	struct MoveEntity
	{
		WpEntity wpEntity;
	};
	struct 开始采集晶体矿
	{
	};
	struct 晶体矿已运回基地
	{
	};
	struct 单位阵亡
	{
		WpEntity wpEntity;
	};
	bool SameSpace(const WpEntity& refWpEntity, const Space& refSpace, PlayerGateSession_Game& refGateSession);
}

