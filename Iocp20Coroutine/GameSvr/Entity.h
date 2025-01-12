#pragma once
#include "../CoRoutine/CoTask.h"
#include <functional>
#include "MyMsgQueue.h"
#include "SpEntity.h"
#include "单位.h"

//#include "PlayerComponent.h"
class Space;
class GameSvrSession;
class GameSvr;
class PlayerComponent;
class PlayerNickNameComponent;
class MonsterComponent;
class BuildingComponent;
class AttackComponent;
class DefenceComponent;
class 采集Component;
class 资源Component;
class 地堡Component;
class 走Component;
class 临时阻挡Component;
class 造活动单位Component;
class 造建筑Component;
class AoiComponent;
class PlayerGateSession_Game;


class Entity final : public std::enable_shared_from_this<Entity>//必须公有继承，否则无效
{
public:
	Entity(const Position& pos, Space& refSpace, 单位类型 类型, const 单位::单位配置& ref配置);
	Entity(const Entity&) = delete;
	~Entity();
	void Save(std::ofstream& refOf);
	static bool Load(Space& refSpace, char(&buf)[1024], const uint16_t u16Size);
	void Update();
	bool IsDead()const;
	bool NeedDelete()const { return m_bNeedDelete; }
	bool DistanceLessEqual(const Entity& refEntity, float fDistance);
	float DistancePow2(const Entity& refEntity)const;
	float Distance(const Entity& refEntity)const;
	void OnDestroy();
	void BroadcastLeave();
	const std::string& NickName();
	void BroadcastEnter();
	void BroadcastNotifyPos();
	void BroadcastChangeSkeleAnim(const std::string& refAniClipName, bool loop = true);
	CoTaskBool CoDelayDelete();
	template<class T> void Broadcast(const T& msg);
	bool IsEnemy(const Entity& refEntity);
	const Position& Pos()const { return m_Pos; }
	void SetPos(const Position& refNewPos);
	int m_eulerAnglesY = 0;
	//CoTask<int> m_coWaitDelete;
	FunCancel m_cancelDelete;

	const uint64_t Id;
	bool m_bNeedDelete = false;
	float m_速度每帧移动距离 = 0.5f;//每帧多少米
	float 攻击距离()const;
	float 警戒距离()const;

	const 单位类型 m_类型;
	单位::单位配置 m_配置;

	//静态ECS，没有基类强转子类
	std::shared_ptr<PlayerComponent> m_spPlayer;
	std::shared_ptr<PlayerNickNameComponent> m_spPlayerNickName;
	std::shared_ptr<AttackComponent> m_spAttack;
	std::shared_ptr<DefenceComponent> m_spDefence;
	std::shared_ptr<MonsterComponent> m_spMonster;
	std::shared_ptr<BuildingComponent> m_spBuilding;
	std::shared_ptr<采集Component> m_sp采集;
	std::shared_ptr<资源Component> m_sp资源;
	std::shared_ptr<地堡Component> m_sp地堡;
	std::shared_ptr<走Component> m_sp走;
	std::shared_ptr<临时阻挡Component> m_sp临时阻挡;
	std::shared_ptr<造活动单位Component> m_sp造活动单位;
	std::shared_ptr<造建筑Component> m_sp造建筑;
	std::unique_ptr<AoiComponent> m_upAoi;
	/// <summary>
	/// 地堡或运输机
	/// </summary>
	WpEntity m_wpOwner;
	//private:
	Space& m_refSpace;
private:
	Position m_Pos;

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

