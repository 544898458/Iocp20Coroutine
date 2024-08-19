#pragma once
#include "../CoRoutine/CoTask.h"
#include "MyMsgQueue.h"
class Entity;
class AttackComponent final
{
public:
	void WalkToPos(Entity& refEntity, const Position& posTarget);
	void TryCancel(Entity& refEntity);
	void Update(Entity& refEntity);

	CoTask<int> m_coWalk;
	CoTask<int> m_coAttack;
	FunCancel m_cancel;
};

