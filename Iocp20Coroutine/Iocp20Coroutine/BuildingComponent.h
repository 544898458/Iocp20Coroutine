#pragma once
#include "../CoRoutine/CoTask.h"
#include "SpEntity.h"
class Entity;
class GameSvrSession;
class PlayerGateSession_Game;
enum ���λ����;
enum ������λ����;
class BuildingComponent
{
public:
	BuildingComponent(PlayerGateSession_Game& refSession, const ������λ���� &����);
	void TryCancel(Entity& refEntity);
	void ���(PlayerGateSession_Game&, Entity& refEntity);
	typedef CoTaskBool(*Fun���)(BuildingComponent& refThis, PlayerGateSession_Game& refGateSession, Entity& refEntity);
	Fun��� m_fun����λ;
	const ������λ���� m_����;
	static CoTaskBool Co���(BuildingComponent& refThis, PlayerGateSession_Game& refGateSession, Entity& refEntity);
	static CoTaskBool Co�칤�̳�(BuildingComponent& refThis, PlayerGateSession_Game& refGateSession, Entity& refEntity);
	static void AddComponent(Entity& refThis, PlayerGateSession_Game& refSession, const ������λ���� ����);
private:
	static CoTask<SpEntity> Co����λ(BuildingComponent& refThis, PlayerGateSession_Game& refGateSession, Entity& refEntity, const ���λ���� ����);
	int m_i�ȴ������ = 0;
	//CoTask<int> m_coAddMoney;
	//FunCancel m_cancelAddMoney;
	CoTaskCancel m_TaskCancel���;
};

