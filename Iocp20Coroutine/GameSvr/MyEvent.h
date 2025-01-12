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
	struct 玩家进入Space
	{
		PlayerGateSession_Game& refPlayerGateSession;
		Entity& ref视口;
		Space& refSpace;
	};
	bool SameSpace(const WpEntity& refWpEntity, const Space& refSpace, const std::string& stdNickName);
}

