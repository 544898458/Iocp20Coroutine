#pragma once
#include "../CoRoutine/CoTask.h"
#include "SpEntity.h"
class Entity;
class GameSvrSession;
class PlayerGateSession_Game;
enum ���λ����;
enum ������λ����;
const int MAX����ٷֱ� = 100;
class BuildingComponent
{
public:
	BuildingComponent(PlayerGateSession_Game& refSession, const ������λ����& ����, Entity& refEntity);
	//typedef CoTaskBool(*Fun���)(BuildingComponent& refThis, PlayerGateSession_Game& refGateSession, Entity& refEntity, const ���λ���� ����);
	//Fun��� m_fun����λ;
	const ������λ���� m_����;
	static void AddComponent(Entity& refThis, PlayerGateSession_Game& refSession, const ������λ���� ����, float f��߳�);
	CoTaskBool Co�������(FunCancel& cancel);
	int m_n������Ȱٷֱ� = 0;
	bool �����()const;
private:

	//CoTask<int> m_coAddMoney;
	FunCancel m_cancel����;

	Entity& m_refEntity;
};

