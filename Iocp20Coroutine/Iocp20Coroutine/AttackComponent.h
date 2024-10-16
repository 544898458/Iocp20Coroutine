#pragma once
#include "../CoRoutine/CoTask.h"
#include "MyMsgQueue.h"
#include "SpEntity.h"
class Entity;
class AttackComponent final
{
public:
	void WalkToPos(Entity& refEntity, const Position& posTarget);
	void TryCancel(Entity& refEntity);
	void Update(Entity& refEntity);
	static void AddComponent(Entity& refEntity);

	CoTaskBool m_coWalk;
	CoTask<int> m_coAttack;
	FunCancel m_cancel;
	static const int INVALID_AGENT_IDX = -1;//0是合法值
	int m_idxCrowdAgent = INVALID_AGENT_IDX;
};

