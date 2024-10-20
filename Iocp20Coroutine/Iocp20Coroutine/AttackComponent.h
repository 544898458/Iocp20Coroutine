#pragma once
#include "../CoRoutine/CoTask.h"
#include "MyMsgQueue.h"
#include "SpEntity.h"
class Entity;
class AttackComponent final
{
public:
	AttackComponent(Entity& refEntity, const ���λ���� ����);
	void WalkToPos�ֶ�����(const Position& posTarget);
	void TryCancel();
	void Update();
	static void AddComponent(Entity& refEntity, const ���λ���� ����);

	CoTaskBool m_coWalk;
	CoTaskBool m_coWalk�ֶ�����;
	CoTask<int> m_coAttack;
	FunCancel m_cancel;//�ߺʹ򲻿�ͬʱ���ڣ���˹���һ��Cancel
	static const int INVALID_AGENT_IDX = -1;//0�ǺϷ�ֵ
	int m_idxCrowdAgent = INVALID_AGENT_IDX;
	Entity& m_refEntity;
	const ���λ���� m_����;
};

