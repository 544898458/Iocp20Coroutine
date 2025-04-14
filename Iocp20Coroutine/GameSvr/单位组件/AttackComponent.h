#pragma once
#include "../CoRoutine/CoTask.h"
#include "MyMsgQueue.h"
#include "SpEntity.h"
#include "单位.h"
class Entity;
class 找目标走过去Component;
class AttackComponent final
{
public:
	AttackComponent(Entity& refEntity);
	void TryCancel(const bool bDestroy = false);
	void Update();
	CoTaskBool CoAttack目标(WpEntity wpDefencer, FunCancel& cancel);
	CoTaskBool CoAttack位置(const Position pos, const float f目标建筑半边长, FunCancel& cancel);
	void 处理仇恨目标(WpEntity& wpEntity, bool& ref仇恨目标);
	static AttackComponent& AddComponent(Entity& refEntity);
	FunCancel m_cancelAttack;
	Entity& m_refEntity;
private:
	CoTask<std::tuple<bool, bool>> Co攻击(const Entity& refTarget, WpEntity wpEntity, 找目标走过去Component& ref找目标走过去);
	WpEntity Get最近的敌人();
	bool 可以攻击();
};

