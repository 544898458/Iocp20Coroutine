#pragma once
#include "../CoRoutine/CoTask.h"
#include <functional>
#include "MyMsgQueue.h"
//#include "PlayerComponent.h"
class Space;
class MySession;
class MyServer;
class PlayerComponent;
class Entity :public std::enable_shared_from_this<Entity>//必须公有继承，否则无效
{
public:
	Entity();
	Entity(const Entity&) = delete;
	void Init(const Position& pos, Space& m_space, const std::string& strPrefabName);
	void WalkToPos(const Position& posTarget, MyServer* pServer);
	void Update();
	void TryCancel();
	void Hurt(int);
	bool IsDead()const { return m_hp <= 0; }
	bool DistanceLessEqual(const Entity& refEntity, float fDistance);
	void OnDestroy();
	const std::string& NickName();
	template<class T> void Broadcast(const T& msg);
	bool IsEnemy(const Entity& refEntity);
	Position m_Pos;
	int m_eulerAnglesY = 0;
	CoTask<int> m_coWalk;
	CoTask<int> m_coIdle;
	CoTask<int> m_coAttack;
	//std::function<void()> m_cancelAttack;
	std::function<void()> m_cancel;
	const uint64_t Id;
	int m_hp = 20;
	float m_f警戒距离 = 30;
	float m_f移动速度 = 0.5f;//每帧多少米
	const float m_f攻击距离 = 5.0f;
	
	std::string m_strPrefabName;

	//静态ECS，没有基类向子类转型
	void AddComponent(MySession* pSession);
	std::shared_ptr<PlayerComponent> m_spPlayer;
	//private:
	Space* m_space;
};

//x,y就是坐标系中的坐标，如（4，4）答案就是45°
inline int CalculateAngle(float x, float y)
{
	auto angleRad = std::atan2(x, y); // 计算弧度
	auto angleDeg = angleRad * 180.0f / 3.14159265f; // 将弧度转换为角度
	return (int)angleDeg; // 返回角度
}

inline int CalculateAngle(Position from, Position to)
{
	return CalculateAngle(to.x - from.x, to.z - from.z);
}

