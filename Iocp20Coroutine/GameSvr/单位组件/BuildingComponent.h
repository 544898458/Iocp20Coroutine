#pragma once
#include "../CoRoutine/CoTask.h"
#include "SpEntity.h"
class Entity;
class GameSvrSession;
class PlayerGateSession_Game;
enum 单位类型;
enum 单位类型;
const int MAX建造百分比 = 100;
class BuildingComponent
{
public:
	BuildingComponent(Entity& refEntity);
	static void AddComponent(Entity& refThis, float f半边长);
	CoTaskBool Co建造过程(FunCancel& cancel);
	int m_n建造进度百分比 = 0;
	bool 已造好()const;
	static float 建筑半边长(const Entity& refEntity);
private:

	//CoTask<int> m_coAddMoney;
	FunCancel m_cancel建造;

	Entity& m_refEntity;
};

