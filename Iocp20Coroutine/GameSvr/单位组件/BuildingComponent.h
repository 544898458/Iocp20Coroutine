#pragma once
#include "../CoRoutine/CoTask.h"
#include "SpEntity.h"
class Entity;
class GameSvrSession;
class PlayerGateSession_Game;
enum 单位类型;
enum 单位类型;
class BuildingComponent
{
public:
	BuildingComponent(Entity& refEntity);
	static void AddComponent(Entity& refThis, float f半边长);
	void StartCo建造过程();
	void 直接造好();
	void OnEntityDestroy(const bool bDestroy);
	int m_n建造进度帧 = 0;
	bool 已造好()const;
	static float 建筑半边长(const Entity& refEntity);
	static const int MAX建造十分比 = 10;
private:
	CoTaskBool Co建造过程(FunCancel& cancel);

	//CoTask<int> m_coAddMoney;
	FunCancel安全 m_cancel建造;

	Entity& m_refEntity;
};

