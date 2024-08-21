#pragma once
#include "../CoRoutine/CoTask.h"
class Entity;
class BuildingComponent
{
public:
	BuildingComponent(Entity& refEntity);
	void TryCancel(Entity& refEntity);
private:
	CoTask<int> m_coAddMoney;
	FunCancel m_cancel;
};

