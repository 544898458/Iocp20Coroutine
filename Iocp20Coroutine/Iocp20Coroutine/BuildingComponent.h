#pragma once
#include "../CoRoutine/CoTask.h"
class Entity;
class GameSvrSession;
class PlayerGateSession_Game;
enum 活动单位类型;
class BuildingComponent
{
public:
	BuildingComponent(PlayerGateSession_Game& refSession);
	void TryCancel(Entity& refEntity);
	void 造兵(PlayerGateSession_Game&, Entity& refEntity);
	typedef CoTaskUint8(*Fun造兵)(BuildingComponent& refThis, PlayerGateSession_Game& refGateSession, Entity& refEntity);
	Fun造兵 m_fun造活动单位;
	static CoTaskUint8 Co造兵(BuildingComponent& refThis, PlayerGateSession_Game& refGateSession, Entity& refEntity);
	static CoTaskUint8 Co造工程车(BuildingComponent& refThis, PlayerGateSession_Game& refGateSession, Entity& refEntity);
private:
	static CoTaskUint8 Co造活动单位(BuildingComponent& refThis, PlayerGateSession_Game& refGateSession, Entity& refEntity, const 活动单位类型 类型);
	int m_i等待造兵数 = 0;
	//CoTask<int> m_coAddMoney;
	//FunCancel m_cancelAddMoney;
	CoTaskCancel m_TaskCancel造兵;
};

