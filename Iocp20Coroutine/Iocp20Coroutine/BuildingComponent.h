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
	BuildingComponent(PlayerGateSession_Game& refSession, const ������λ���� &����, Entity &refEntity);
	void TryCancel(Entity& refEntity);
	void ���(PlayerGateSession_Game&, Entity& refEntity, const ���λ���� ����);
	//typedef CoTaskBool(*Fun���)(BuildingComponent& refThis, PlayerGateSession_Game& refGateSession, Entity& refEntity, const ���λ���� ����);
	//Fun��� m_fun����λ;
	const ������λ���� m_����;
	static CoTaskBool Co���(BuildingComponent& refThis, PlayerGateSession_Game& refGateSession, Entity& refEntity, const ���λ���� ����);
	static CoTaskBool Co�칤�̳�(BuildingComponent& refThis, PlayerGateSession_Game& refGateSession, Entity& refEntity);
	static void AddComponent(Entity& refThis, PlayerGateSession_Game& refSession, const ������λ���� ����, float f��߳�);
	int m_i�ȴ������ = 0;
private:
	static CoTaskBool Co����λ(BuildingComponent& refThis, PlayerGateSession_Game& refGateSession, Entity& refEntity, const ���λ���� ����, std::function<void(Entity&)> fun = {});
	
	//CoTask<int> m_coAddMoney;
	//FunCancel m_cancelAddMoney;
	Entity& m_refEntity;
	CoTaskCancel m_TaskCancel���;
};

