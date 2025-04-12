#pragma once
#include "../CoRoutine/CoTask.h"
#include "MyMsgQueue.h"
#include "SpEntity.h"
#include "��λ.h"
class Entity;
class AttackComponent final
{
public:
	AttackComponent(Entity& refEntity);
	static Position �����й�(const Position& refOld);
	void TryCancel(const bool bDestroy = false);
	void Update();
	CoTaskBool Co���򾯽䷶Χ�ڵ�Ŀ��Ȼ�󹥻�();
	void ����ǰҡ����();
	void ���Ź�������();
	void ���Ź�����Ч();
	CoTaskBool CoAttackĿ��(WpEntity wpDefencer, FunCancel& cancel);
	static uint16_t ������Ĺ���(Entity& refEntity);
	CoTaskBool CoAttackλ��(const Position pos, const float fĿ�꽨����߳�, FunCancel& cancel);
	static AttackComponent& AddComponent(Entity& refEntity);
	float ��������(const Entity& refTarget)const;
	float ��������(const float fĿ�꽨����߳�) const;
	//CoTask<int> m_coAttack;
	FunCancel m_cancelAttack;
	Entity& m_refEntity;
	CoTaskCancel m_TaskCancel;
	using Fun��������Ŀ�� = std::function<Position(const Position&)>;
	Fun��������Ŀ�� m_fun��������˴�;
	��λ::ս������ m_ս������;
	FunCancel m_funCancel����;
	bool m_b�����µ�Ŀ�� = true;
	bool m_bԭ�ؼ��� = false;//Hold Position
private:
	bool ���Թ���();
	bool ��鴩ǽ(const Entity& refEntity);
	CoTaskBool Co����();
};

