#pragma once
#include "../CoRoutine/CoTask.h"
#include "SpEntity.h"
#include <unordered_set>
class Entity;
class GameSvrSession;
class PlayerGateSession_Game;
enum 活动单位类型;
enum 建筑单位类型;

class BuildingComponent
{
public:
	BuildingComponent(PlayerGateSession_Game& refSession, const 建筑单位类型 &类型, Entity &refEntity);
	void TryCancel(Entity& refEntity);
	void 造兵(PlayerGateSession_Game&, Entity& refEntity, const 活动单位类型 类型);
	//typedef CoTaskBool(*Fun造兵)(BuildingComponent& refThis, PlayerGateSession_Game& refGateSession, Entity& refEntity, const 活动单位类型 类型);
	//Fun造兵 m_fun造活动单位;
	const 建筑单位类型 m_类型;
	static void AddComponent(Entity& refThis, PlayerGateSession_Game& refSession, const 建筑单位类型 类型, float f半边长);
	
	std::unordered_set<活动单位类型> m_set可造类型;
	std::list<活动单位类型> m_list等待造;//int m_i等待造兵数 = 0;
private:
	static CoTaskBool Co造活动单位(BuildingComponent& refThis, PlayerGateSession_Game& refGateSession, Entity& refEntity);
	
	//CoTask<int> m_coAddMoney;
	//FunCancel m_cancelAddMoney;
	Entity& m_refEntity;
	CoTaskCancel m_TaskCancel造兵;
};

