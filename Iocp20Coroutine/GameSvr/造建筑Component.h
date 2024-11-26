#pragma once
#include <unordered_set>
#include "../CoRoutine/CoTask.h"

class PlayerGateSession_Game;
class Entity;
enum 建筑单位类型;
enum 活动单位类型;
struct Position;

/// <summary>
/// 此单位可以造建筑（工程车、工蜂、探机）
/// </summary>
class 造建筑Component
{
public:
	static void AddComponent(Entity& refEntity, PlayerGateSession_Game& refGateSession, const 活动单位类型 类型);
	造建筑Component(PlayerGateSession_Game& refSession, Entity& refEntity, const 活动单位类型 类型);
	CoTaskBool Co造建筑(const Position refPos, const 建筑单位类型 类型);
	CoTaskBool Co建造过程(WpEntity wpEntity建筑, FunCancel& cancel);
private:
	std::unordered_set<建筑单位类型> m_set可造类型;
	FunCancel m_cancel造建筑;
	Entity& m_refEntity;
};

