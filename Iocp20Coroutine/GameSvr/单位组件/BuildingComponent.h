#pragma once
#include "../CoRoutine/CoTask.h"
#include "SpEntity.h"
class Entity;
class GameSvrSession;
class PlayerGateSession_Game;
enum ��λ����;
enum ��λ����;
class BuildingComponent
{
public:
	BuildingComponent(Entity& refEntity);
	static void AddComponent(Entity& refThis, float f��߳�);
	void StartCo�������();
	void ֱ�����();
	void OnEntityDestroy(const bool bDestroy);
	int m_n������Ȱٷֱ� = 0;
	bool �����()const;
	static float ������߳�(const Entity& refEntity);
	static const int MAX����ٷֱ� = 100;
private:
	CoTaskBool Co�������(FunCancel& cancel);

	//CoTask<int> m_coAddMoney;
	FunCancel��ȫ m_cancel����;

	Entity& m_refEntity;
};

