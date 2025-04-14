#pragma once
#include "../CoRoutine/CoTask.h"
#include "MyMsgQueue.h"
#include "SpEntity.h"
#include "��λ.h"
class Entity;
class ��Ŀ���߹�ȥComponent;
class AttackComponent final
{
public:
	AttackComponent(Entity& refEntity);
	void TryCancel(const bool bDestroy = false);
	void Update();
	CoTaskBool CoAttackĿ��(WpEntity wpDefencer, FunCancel& cancel);
	CoTaskBool CoAttackλ��(const Position pos, const float fĿ�꽨����߳�, FunCancel& cancel);
	void ������Ŀ��(WpEntity& wpEntity, bool& ref���Ŀ��);
	static AttackComponent& AddComponent(Entity& refEntity);
	FunCancel m_cancelAttack;
	Entity& m_refEntity;
private:
	CoTask<std::tuple<bool, bool>> Co����(const Entity& refTarget, WpEntity wpEntity, ��Ŀ���߹�ȥComponent& ref��Ŀ���߹�ȥ);
	WpEntity Get����ĵ���();
	bool ���Թ���();
};

