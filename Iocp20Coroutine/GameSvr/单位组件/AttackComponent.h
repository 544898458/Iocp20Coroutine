#pragma once
#include "../CoRoutine/CoTask.h"
#include "MyMsgQueue.h"
#include "SpEntity.h"
#include "单位.h"
class Entity;
class AttackComponent final
{
public:
	AttackComponent(Entity& refEntity);
	static Position 怪物闲逛(const Position& refOld);
	void TryCancel(const bool bDestroy = false);
	void Update();
	CoTaskBool Co走向警戒范围内的目标然后攻击();
	void 播放前摇动作();
	void 播放攻击动作();
	void 播放攻击音效();
	CoTaskBool CoAttack目标(WpEntity wpDefencer, FunCancel& cancel);
	static uint16_t 升级后的攻击(Entity& refEntity);
	CoTaskBool CoAttack位置(const Position pos, const float f目标建筑半边长, FunCancel& cancel);
	static AttackComponent& AddComponent(Entity& refEntity);
	float 攻击距离(const Entity& refTarget)const;
	float 攻击距离(const float f目标建筑半边长) const;
	//CoTask<int> m_coAttack;
	FunCancel m_cancelAttack;
	Entity& m_refEntity;
	CoTaskCancel m_TaskCancel;
	using Fun空闲走向目标 = std::function<Position(const Position&)>;
	Fun空闲走向目标 m_fun空闲走向此处;
	单位::战斗配置 m_战斗配置;
	FunCancel m_funCancel顶层;
	bool m_b搜索新的目标 = true;
	bool m_b原地坚守 = false;//Hold Position
private:
	bool 可以攻击();
	bool 检查穿墙(const Entity& refEntity);
	CoTaskBool Co顶层();
};

