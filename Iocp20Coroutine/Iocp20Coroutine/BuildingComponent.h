#pragma once
#include "../CoRoutine/CoTask.h"
class Entity;
class GameSvrSession;
class PlayerGateSession_Game;
enum ���λ����;
class BuildingComponent
{
public:
	BuildingComponent(PlayerGateSession_Game& refSession);
	void TryCancel(Entity& refEntity);
	void ���(PlayerGateSession_Game&, Entity& refEntity);
	typedef CoTaskUint8(*Fun���)(BuildingComponent& refThis, PlayerGateSession_Game& refGateSession, Entity& refEntity);
	Fun��� m_fun����λ;
	static CoTaskUint8 Co���(BuildingComponent& refThis, PlayerGateSession_Game& refGateSession, Entity& refEntity);
	static CoTaskUint8 Co�칤�̳�(BuildingComponent& refThis, PlayerGateSession_Game& refGateSession, Entity& refEntity);
private:
	static CoTaskUint8 Co����λ(BuildingComponent& refThis, PlayerGateSession_Game& refGateSession, Entity& refEntity, const ���λ���� ����);
	int m_i�ȴ������ = 0;
	//CoTask<int> m_coAddMoney;
	//FunCancel m_cancelAddMoney;
	CoTaskCancel m_TaskCancel���;
};

