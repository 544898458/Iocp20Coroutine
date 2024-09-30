#pragma once
#include "../CoRoutine/CoTask.h"
#include <functional>
#include "MyMsgQueue.h"
//#include "PlayerComponent.h"
class Space;
class GameSvrSession;
class GameSvr;
class PlayerComponent;
class MonsterComponent;
class BuildingComponent;
class AttackComponent;
class PlayerGateSession_Game;
class Entity :public std::enable_shared_from_this<Entity>//���빫�м̳У�������Ч
{
public:
	Entity(const Position& pos, Space& m_space, const std::string& strPrefabName);
	Entity(const Entity&) = delete;
	void Update();
	void Hurt(int);
	bool IsDead()const { return m_hp <= 0; }
	bool NeedDelete()const { return m_bNeedDelete; }
	bool DistanceLessEqual(const Entity& refEntity, float fDistance);
	float DistancePow2(const Entity& refEntity)const;
	void OnDestroy();
	const std::string& NickName();
	void BroadcastEnter();
	template<class T> void Broadcast(const T& msg);
	bool IsEnemy(const Entity& refEntity);
	Position m_Pos;
	int m_eulerAnglesY = 0;
	CoTask<int> m_coWaitDelete;
	FunCancel m_cancelDelete;
	
	const uint64_t Id;
	bool m_bNeedDelete = false;
	int m_hp = 20;
	float m_f������� = 30;
	float m_f�ƶ��ٶ� = 0.5f;//ÿ֡������
	const float m_f�������� = 5.0f;

	std::string m_strPrefabName;

	//��̬ECS��û�л���ǿת����
	void AddComponentPlayer(PlayerGateSession_Game& refSession);
	void AddComponentAttack();
	void AddComponentMonster();
	void AddComponentBuilding(PlayerGateSession_Game& refSession);
	std::shared_ptr<PlayerComponent> m_spPlayer;
	std::shared_ptr<AttackComponent> m_spAttack;
	std::shared_ptr<MonsterComponent> m_spMonster;
	std::shared_ptr<BuildingComponent> m_spBuilding;
	//private:
	Space& m_space;
};

//x,y��������ϵ�е����꣬�磨4��4���𰸾���45��
inline int CalculateAngle(float x, float y)
{
	auto angleRad = std::atan2(x, y); // ���㻡��
	auto angleDeg = angleRad * 180.0f / 3.14159265f; // ������ת��Ϊ�Ƕ�
	return (int)angleDeg; // ���ؽǶ�
}

inline int CalculateAngle(Position from, Position to)
{
	return CalculateAngle(to.x - from.x, to.z - from.z);
}

