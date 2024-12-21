#pragma once
#include "../CoRoutine/CoTask.h"
#include "MyMsgQueue.h"
#include "SpEntity.h"
class Entity;
class AttackComponent final
{
public:
	AttackComponent(Entity& refEntity, const ���λ���� ����);
	void TryCancel(const bool bDestroy=false);
	void Update();
	CoTaskBool Co���򾯽䷶Χ�ڵ�Ŀ��Ȼ�󹥻�(FunCancel& funCancel);
	CoTaskBool CoAttack(WpEntity wpDefencer, FunCancel& cancel);
	static void AddComponent(Entity& refEntity, const ���λ���� ����, const float f��������, const float f�˺�, const float f�������);
	float ��������(const Entity& refTarget)const;
	//CoTask<int> m_coAttack;
	FunCancel m_cancelAttack;
	static const int INVALID_AGENT_IDX = -1;//0�ǺϷ�ֵ
	int m_idxCrowdAgent = INVALID_AGENT_IDX;
	Entity& m_refEntity;
	const ���λ���� m_����;
	float m_f�������� = 5.0f;
	float m_f�˺� = 3;
	CoTaskCancel m_TaskCancel;
	typedef Position (*Fun��������Ŀ��)(const Position&);
	Fun��������Ŀ�� m_fun��������˴�;
	float m_f������� = 30;
	FunCancel m_funCancel����;
	bool m_b�����µ�Ŀ�� = true;
private:
	bool ���Թ���();
	CoTaskBool Co����();
};

