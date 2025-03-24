#pragma once
#include "SpEntity.h"
#include "../CoRoutine/CoTask.h"

struct Position;



class 走Component
{
public:
	走Component(Entity& refEntity);
	void WalkToPos手动控制(const Position& posTarget);
	void WalkToTarget(SpEntity spTarget);
	void WalkToPos(const Position& posTarget);
	void TryCancel();
	void 走进地堡(WpEntity wpEntity地堡);
	static void AddComponent(Entity& refEntity);
	static void Cancel所有包含走路的协程(Entity& refEntity, const bool b停止攻击 = false);
	static bool 正在走(const Entity& refEntity);
	static bool WalkToTarget(Entity& refThis, SpEntity spTarget);
	static bool WalkToPos(Entity& refThis, const Position& posTarget);
	static const int INVALID_AGENT_IDX = -1;//0是合法值
	int m_idxCrowdAgent = INVALID_AGENT_IDX;

	CoTaskBool m_coWalk;
	CoTaskBool m_coWalk手动控制;
	CoTaskBool m_coWalk进地堡;
	FunCancel m_cancel;

private:
	CoTaskBool Co走进地堡(WpEntity wpEntity地堡);

	Entity& m_refEntity;
};

