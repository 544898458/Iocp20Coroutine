#pragma once
#include "SpEntity.h"
#include "../CoRoutine/CoTask.h"

struct Position;



class 走Component
{
public:
	走Component(Entity& refEntity);
	static void AddComponent(Entity& refEntity);
	static bool 正在走(Entity& refEntity);
	void WalkToPos手动控制(const Position& posTarget);
	void WalkToTarget(SpEntity spTarget);
	void WalkToPos(const Position& posTarget);
	static bool WalkToTarget(Entity& refThis, SpEntity spTarget);
	static bool WalkToPos(Entity& refThis, const Position& posTarget);
	void TryCancel();

	CoTaskBool m_coWalk;
	CoTaskBool m_coWalk手动控制;

private:
	Entity& m_refEntity;
	FunCancel m_cancel;
};

