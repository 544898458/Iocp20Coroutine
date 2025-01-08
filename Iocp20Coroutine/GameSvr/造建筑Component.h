#pragma once
#include <unordered_set>
#include "../CoRoutine/CoTask.h"
#include "SpEntity.h"

class PlayerGateSession_Game;
class Entity;
enum 单位类型;
enum 单位类型;
struct Position;

/// <summary>
/// 此单位可以造建筑（工程车、工蜂、探机）
/// </summary>
class 造建筑Component final
{
public:
	static void AddComponent(Entity& refEntity, const 单位类型 类型);
	static bool 正在建造(Entity& refEntity);
	造建筑Component(Entity& refEntity, const 单位类型 类型);
	~造建筑Component();
	CoTaskBool Co造建筑(const Position refPos, const 单位类型 类型);
	void TryCancel();
private:
	CoTaskBool Co建造过程(WpEntity wpEntity建筑, FunCancel& cancel);
	CoTask<SpEntity> CoAddBuilding(const 单位类型 类型, const Position pos);
	std::unordered_set<单位类型> m_set可造类型;
	FunCancel m_cancel造建筑;
	Entity& m_refEntity;
};

