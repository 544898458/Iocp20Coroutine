#pragma once
#include "../CoRoutine/CoTask.h"
#include "SpEntity.h"
class Entity;
class GameSvrSession;
class PlayerGateSession_Game;
enum 活动单位类型;
enum 建筑单位类型;
const int MAX建造百分比 = 100;
class BuildingComponent
{
public:
	BuildingComponent(const 建筑单位类型& 类型, Entity& refEntity);
	//typedef CoTaskBool(*Fun造兵)(BuildingComponent& refThis, PlayerGateSession_Game& refGateSession, Entity& refEntity, const 活动单位类型 类型);
	//Fun造兵 m_fun造活动单位;
	const 建筑单位类型 m_类型;
	static void AddComponent(Entity& refThis, const 建筑单位类型 类型, float f半边长);
	CoTaskBool Co建造过程(FunCancel& cancel);
	int m_n建造进度百分比 = 0;
	bool 已造好()const;
	static float 建筑半边长(const Entity& refEntity);
private:

	//CoTask<int> m_coAddMoney;
	FunCancel m_cancel建造;

	Entity& m_refEntity;
};

