#pragma once
#include "../CoRoutine/CoTask.h"
#include "MyMsgQueue.h"
#include "SpEntity.h"
#include "��λ.h"
class Entity;
class AttackComponent final
{
public:
	AttackComponent(Entity& refEntity, const ���λ���� ����, const ��λ::ս������& ����);
	void TryCancel(const bool bDestroy = false);
	void Update();
	CoTaskBool Co���򾯽䷶Χ�ڵ�Ŀ��Ȼ�󹥻�(FunCancel& funCancel);
	void ����ǰҡ����();
	void ���Ź�������();
	void ���Ź�����Ч();
	CoTaskBool CoAttackĿ��(WpEntity wpDefencer, FunCancel& cancel);
	CoTaskBool CoAttackλ��(const Position pos, FunCancel& cancel);
	static void AddComponent(Entity& refEntity, const ���λ���� ����, const ��λ::ս������& ����);
	float ��������(const Entity& refTarget)const;
	float ��������() const;
	//CoTask<int> m_coAttack;
	FunCancel m_cancelAttack;
	static const int INVALID_AGENT_IDX = -1;//0�ǺϷ�ֵ
	int m_idxCrowdAgent = INVALID_AGENT_IDX;
	Entity& m_refEntity;
	const ���λ���� m_����;
	CoTaskCancel m_TaskCancel;
	typedef Position(*Fun��������Ŀ��)(const Position&);
	Fun��������Ŀ�� m_fun��������˴�;
	��λ::ս������ m_ս������;
	FunCancel m_funCancel����;
	bool m_b�����µ�Ŀ�� = true;
private:
	bool ���Թ���();
	CoTaskBool Co����();
};

