#pragma once
#include "../CoRoutine/CoTask.h"
#include "MyMsgQueue.h"
#include "SpEntity.h"
class Entity;
class AttackComponent final
{
public:
	AttackComponent(Entity& refEntity, const 活动单位类型 类型);
	void TryCancel();
	void Update();
	CoTask<int> CoAttack(WpEntity wpDefencer, FunCancel& cancel);
	static void AddComponent(Entity& refEntity, const 活动单位类型 类型, const float f攻击距离);
	float 攻击距离()const;
	CoTask<int> m_coAttack;
	FunCancel m_cancel;
	static const int INVALID_AGENT_IDX = -1;//0是合法值
	int m_idxCrowdAgent = INVALID_AGENT_IDX;
	Entity& m_refEntity;
	const 活动单位类型 m_类型;
	float m_f攻击距离 = 5.0f;
};

