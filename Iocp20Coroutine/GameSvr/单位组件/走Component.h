#pragma once
#include "SpEntity.h"
#include "../CoRoutine/CoTask.h"

struct Position;



class ��Component
{
public:
	��Component(Entity& refEntity);
	void WalkToPos�ֶ�����(const Position& posTarget);
	void WalkToTarget(SpEntity spTarget);
	void WalkToPos(const Position& posTarget);
	void TryCancel();
	void �߽��ر�(WpEntity wpEntity�ر�);
	static void AddComponent(Entity& refEntity);
	static void Cancel���а�����·��Э��(Entity& refEntity, const bool bֹͣ���� = false);
	static bool ������(const Entity& refEntity);
	static bool WalkToTarget(Entity& refThis, SpEntity spTarget);
	static bool WalkToPos(Entity& refThis, const Position& posTarget);
	static const int INVALID_AGENT_IDX = -1;//0�ǺϷ�ֵ
	int m_idxCrowdAgent = INVALID_AGENT_IDX;

	CoTaskBool m_coWalk;
	CoTaskBool m_coWalk�ֶ�����;
	CoTaskBool m_coWalk���ر�;
	FunCancel m_cancel;

private:
	CoTaskBool Co�߽��ر�(WpEntity wpEntity�ر�);

	Entity& m_refEntity;
};

