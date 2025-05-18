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
		std::weak_ptr<PlayerGateSession_Game> wpPlayerGateSession;
		std::weak_ptr<Entity> wp视口;
		std::weak_ptr<Space> wpSpace;
	};
	
	struct 已阅读剧情对话
	{
		std::weak_ptr<PlayerGateSession_Game> wpPlayerGateSession;
	};

	struct 进活动单位
	{
		WpEntity wpOwner;
	};

	struct 治疗
	{
		WpEntity wp医疗兵;
	};

	bool SameSpace(const WpEntity& refWpEntity, const Space& refSpace, const std::string& stdNickName);
}

