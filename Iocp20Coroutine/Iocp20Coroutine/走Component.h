#pragma once
#include "SpEntity.h"
#include "../CoRoutine/CoTask.h"

struct Position;



class ��Component
{
public:
	��Component(Entity& refEntity);
	static void AddComponent(Entity& refEntity);
	static bool ������(Entity& refEntity);
	void WalkToPos�ֶ�����(const Position& posTarget);
	void WalkToTarget(SpEntity spTarget);
	void WalkToPos(const Position& posTarget);
	static bool WalkToTarget(Entity& refThis, SpEntity spTarget);
	static bool WalkToPos(Entity& refThis, const Position& posTarget);
	void TryCancel();

	CoTaskBool m_coWalk;
	CoTaskBool m_coWalk�ֶ�����;

private:
	Entity& m_refEntity;
	FunCancel m_cancel;
};

