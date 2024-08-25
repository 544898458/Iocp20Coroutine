#pragma once
#include "../CoRoutine/CoTask.h"
class Entity;
class GameSvrSession;
class PlayerGateSession;
class BuildingComponent
{
public:
	BuildingComponent(PlayerGateSession& refSession);
	void TryCancel(Entity& refEntity);
private:
	CoTask<int> m_coAddMoney;
	FunCancel m_cancel;
};

