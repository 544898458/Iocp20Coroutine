#pragma once
#include "../CoRoutine/CoTask.h"
#include "SpEntity.h"
#include <unordered_set>
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
	static void AddComponent(Entity& refThis, PlayerGateSession_Game& refSession, const ������λ���� ����, float f��߳�);
	
	std::unordered_set<���λ����> m_set��������;
	std::list<���λ����> m_list�ȴ���;//int m_i�ȴ������ = 0;
private:
	static CoTaskBool Co����λ(BuildingComponent& refThis, PlayerGateSession_Game& refGateSession, Entity& refEntity);
	
	//CoTask<int> m_coAddMoney;
	//FunCancel m_cancelAddMoney;
	Entity& m_refEntity;
	CoTaskCancel m_TaskCancel���;
};

