#pragma once
#include "../CoRoutine/CoTask.h"
#include "MyMsgQueue.h"
#include "SpEntity.h"
class Entity;
class AttackComponent final
{
public:
	AttackComponent(Entity& refEntity, const ���λ���� ����);
	void TryCancel();
	void Update();
	CoTask<int> CoAttack(WpEntity wpDefencer, FunCancel& cancel);
	static void AddComponent(Entity& refEntity, const ���λ���� ����);
	CoTask<int> m_coAttack;
	FunCancel m_cancel;
	static const int INVALID_AGENT_IDX = -1;//0�ǺϷ�ֵ
	int m_idxCrowdAgent = INVALID_AGENT_IDX;
	Entity& m_refEntity;
	const ���λ���� m_����;
};

