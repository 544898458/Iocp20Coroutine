#pragma once
#include "../CoRoutine/CoTask.h"
class Entity;
class GameSvrSession;
class PlayerGateSession_Game;
class BuildingComponent
{
public:
	BuildingComponent(PlayerGateSession_Game& refSession);
	void TryCancel(Entity& refEntity);
	void ���(PlayerGateSession_Game&, Entity& refEntity);
private:
	CoTaskUint8 Co���(PlayerGateSession_Game& refGateSession, Entity& refEntity);
private:
	int m_i�ȴ������ = 0;
	//CoTask<int> m_coAddMoney;
	//FunCancel m_cancelAddMoney;
	CoTaskCancel m_TaskCancel���;
};

