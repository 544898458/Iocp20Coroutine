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
	CoTaskBool Co(FunCancel& funCancel);
	CoTaskBool CoAttack(WpEntity wpDefencer, FunCancel& cancel);
	static void AddComponent(Entity& refEntity, const 活动单位类型 类型, const float f攻击距离, const float f伤害, const float f警戒距离);
	float 攻击距离(const Entity& refTarget)const;
	//CoTask<int> m_coAttack;
	FunCancel m_cancelAttack;
	static const int INVALID_AGENT_IDX = -1;//0是合法值
	int m_idxCrowdAgent = INVALID_AGENT_IDX;
	Entity& m_refEntity;
	const 活动单位类型 m_类型;
	float m_f攻击距离 = 5.0f;
	float m_f伤害 = 3;
	CoTaskCancel m_TaskCancel;
	typedef Position (*Fun空闲走向目标)(const Position&);
	Fun空闲走向目标 m_fun空闲走向此处;
	float m_f警戒距离 = 30;

private:
	bool 可以攻击();
};

