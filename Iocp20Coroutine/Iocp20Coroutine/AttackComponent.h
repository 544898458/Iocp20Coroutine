#pragma once
#include "../CoRoutine/CoTask.h"
#include "MyMsgQueue.h"
#include "SpEntity.h"
class Entity;
class AttackComponent final
{
public:
	AttackComponent(Entity& refEntity, const 活动单位类型 类型);
	void WalkToPos手动控制(const Position& posTarget);
	void TryCancel();
	void Update();
	static void AddComponent(Entity& refEntity, const 活动单位类型 类型);

	CoTaskBool m_coWalk;
	CoTaskBool m_coWalk手动控制;
	CoTask<int> m_coAttack;
	FunCancel m_cancel;//走和打不可同时存在，因此共用一个Cancel
	static const int INVALID_AGENT_IDX = -1;//0是合法值
	int m_idxCrowdAgent = INVALID_AGENT_IDX;
	Entity& m_refEntity;
	const 活动单位类型 m_类型;
};

