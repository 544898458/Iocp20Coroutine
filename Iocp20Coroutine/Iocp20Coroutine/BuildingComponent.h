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
	//typedef CoTaskBool(*Fun���)(BuildingComponent& refThis, PlayerGateSession_Game& refGateSession, Entity& refEntity, const ���λ���� ����);
	//Fun��� m_fun����λ;
	const ������λ���� m_����;
	static void AddComponent(Entity& refThis, PlayerGateSession_Game& refSession, const ������λ���� ����, float f��߳�);
	
private:
	//CoTask<int> m_coAddMoney;
	//FunCancel m_cancelAddMoney;
	Entity& m_refEntity;
};

