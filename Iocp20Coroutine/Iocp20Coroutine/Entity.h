#pragma once
#include "../CoRoutine/CoTask.h"
#include <functional>
#include "MyMsgQueue.h"
//#include "PlayerComponent.h"
class Space;
class MySession;
class MyServer;
class PlayerComponent;
class Entity
{
public:
	Entity();
	void Init(float x, Space& m_space, std::function< CoTask<int>(Entity*, float&, float&, std::function<void()>&)> fun, const std::string& strPrefabName);
	void WalkToPos(const float targetX, const float targetZ, MyServer* pServer);
	void Update();
	void TryCancel();
	void Hurt(int);
	bool IsDead()const { return m_hp <= 0; }
	bool DistanceLessEqual(Entity* pEntity, float fDistance);
	void OnDestroy();
	template<class T>
	void Broadcast(const T& msg);
	Position m_Pos;
	int m_eulerAnglesY = 0;
	CoTask<int> m_coWalk;
	CoTask<int> m_coAttack;
	//std::function<void()> m_cancelAttack;
	std::function<void()> m_cancel;
	const uint64_t Id;
	int m_hp = 20;
	const float m_fAttackDistance = 5.0f;
	std::string m_nickName;
	std::string m_strPrefabName;

	//��̬ECS��û�л���������ת��
	void AddComponent(MySession* pSession);
	std::shared_ptr<PlayerComponent> m_spPlayer;
private:
	Space *m_space;
	
};
//x,y��������ϵ�е����꣬�磨4��4���𰸾���45��
inline float CalculateAngle(float x, float y)
{
	auto angleRad = std::atan2(x, y); // ���㻡��
	double angleDeg = angleRad * 180.0f / 3.14159265f; // ������ת��Ϊ��
	return angleDeg; // ���ؽǶ�
}

inline float CalculateAngle(Position from, Position to)
{
	return CalculateAngle(to.x - from.x, to.z - from.z);
}

