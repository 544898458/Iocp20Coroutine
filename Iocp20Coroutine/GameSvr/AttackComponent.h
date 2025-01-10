#pragma once
#include "../CoRoutine/CoTask.h"
#include "MyMsgQueue.h"
#include "SpEntity.h"
#include "单位.h"
class Entity;
class AttackComponent final
{
public:
	AttackComponent(Entity& refEntity, const 单位::战斗配置& 配置);
	static Position 怪物闲逛(const Position& refOld);
	void TryCancel(const bool bDestroy = false);
	void Update();
	CoTaskBool Co走向警戒范围内的目标然后攻击(FunCancel& funCancel);
	void 播放前摇动作();
	void 播放攻击动作();
	void 播放攻击音效();
	CoTaskBool CoAttack目标(WpEntity wpDefencer, FunCancel& cancel);
	CoTaskBool CoAttack位置(const Position pos, const float f目标建筑半边长, FunCancel& cancel);
	static void AddComponent(Entity& refEntity, const 单位类型 类型, const 单位::战斗配置& 配置);
	float 攻击距离(const Entity& refTarget)const;
	float 攻击距离(const float f目标建筑半边长) const;
	//CoTask<int> m_coAttack;
	FunCancel m_cancelAttack;
	static const int INVALID_AGENT_IDX = -1;//0是合法值
	int m_idxCrowdAgent = INVALID_AGENT_IDX;
	Entity& m_refEntity;
	CoTaskCancel m_TaskCancel;
	typedef Position(*Fun空闲走向目标)(const Position&);
	Fun空闲走向目标 m_fun空闲走向此处;
	单位::战斗配置 m_战斗配置;
	FunCancel m_funCancel顶层;
	bool m_b搜索新的目标 = true;
private:
	bool 可以攻击();
	CoTaskBool Co顶层();
};

