#pragma once
#include "../CoRoutine/CoTask.h"
#include <functional>
#include "MyMsgQueue.h"
#include "SpEntity.h"
#include "单位.h"
#include "../proxy/proxy.h"

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
class 虫巢Component;
class 飞向目标Component;
class 解锁单位Component;
class 升级单位属性Component;
class 医疗兵Component;
class 找目标走过去Component;
class BuffComponent;
class 苔蔓Component;
class 苔蔓扩张Component;
class 无苔蔓就持续掉血Component;
class 太岁Component;

PRO_DEF_MEM_DISPATCH(EntityDestroy, OnEntityDestroy);
struct Component : pro::facade_builder
	::add_convention<EntityDestroy, void(bool)>
	::build {};


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
	void OnLoad();
	void BroadcastLeave();
	const std::string& 头顶Name()const;
	void BroadcastEnter();
	void BroadcastNotifyPos();
	void BroadcastChangeSkeleAnim(const std::string& refAniClipName, bool loop = true);
	void DelayDelete(const std::chrono::system_clock::duration& dura = std::chrono::seconds(3));
	template<class T> void Broadcast(const T& msg);
	bool IsEnemy(const Entity& refEntity);
	const Position& Pos()const { return m_Pos; }
	void SetPos(const Position& refNewPos);
	enum FindType
	{
		所有,
		敌方,
		友方,
	};
	WpEntity Get最近的Entity支持地堡中的单位(FindType bFindEnemy, std::function<bool(const Entity&)> fun符合条件);
	WpEntity Get最近的Entity(FindType bFindEnemy, std::function<bool(const Entity&)> fun符合条件);
	WpEntity Get最近的Entity(FindType findType);
	WpEntity Get最近的Entity(const FindType bFindEnemy, const 单位类型 目标类型);
	template<class T>
	bool AddComponentOnDestroy(std::unique_ptr<T> Entity::* pMem, T* pNew)
	{
		CHECK_RET_FALSE(pNew);
		const std::string str组件类名 = typeid(T).name();

		std::unique_ptr<T>& up成员 = this->*pMem;
		if (up成员)
		{
			LOG(ERROR) << "不能重复加" << str组件类名;
			_ASSERT(!"不能重复加组件");
			return false;// *refEntity.m_upAttack;
		}
		up成员.reset(pNew);
		CHECK_RET_FALSE(m_mapComponentOnEntityDstroy.end() == m_mapComponentOnEntityDstroy.find(str组件类名));
		//pro::proxy<Component> pro = pro::make_proxy<Component>(up成员.get());
		m_mapComponentOnEntityDstroy.insert({ str组件类名, pNew });
		return true;
	}

	int m_eulerAnglesY = 0;
	//CoTask<int> m_coWaitDelete;
	FunCancel m_cancelDelete;

	const uint64_t Id;
	bool m_bNeedDelete = false;
	float 攻击距离()const;
	float 警戒距离()const;

	const 单位类型 m_类型;
	单位::单位配置 m_配置;

	//静态ECS，没有基类强转子类
	std::unique_ptr<PlayerComponent> m_upPlayer;
	std::unique_ptr<PlayerNickNameComponent> m_upPlayerNickName;
	std::unique_ptr<AttackComponent> m_upAttack;
	std::unique_ptr<DefenceComponent> m_upDefence;
	//std::unique_ptr<MonsterComponent> m_spMonster;
	std::unique_ptr<BuildingComponent> m_upBuilding;
	std::unique_ptr<采集Component> m_up采集;
	std::unique_ptr<资源Component> m_up资源;
	std::unique_ptr<地堡Component> m_up地堡;
	std::unique_ptr<走Component> m_up走;
	std::unique_ptr<临时阻挡Component> m_up临时阻挡;
	std::unique_ptr<造活动单位Component> m_up造活动单位;
	std::unique_ptr<造建筑Component> m_up造建筑;
	std::unique_ptr<虫巢Component> m_up虫巢;
	std::unique_ptr<AoiComponent> m_upAoi;
	std::unique_ptr<飞向目标Component> m_up飞向目标;
	std::unique_ptr<解锁单位Component> m_up解锁单位;
	std::unique_ptr<升级单位属性Component> m_up升级单位属性;
	std::unique_ptr<医疗兵Component> m_up医疗兵;
	std::unique_ptr<找目标走过去Component> m_up找目标走过去;
	std::unique_ptr<BuffComponent> m_upBuff;
	std::unique_ptr<苔蔓Component> m_up苔蔓;
	std::unique_ptr<苔蔓扩张Component> m_up苔蔓扩张;
	std::unique_ptr<无苔蔓就持续掉血Component> m_up无苔蔓就持续掉血;
	std::unique_ptr<太岁Component> m_up太岁;


	/// <summary>
	/// 地堡或运输机
	/// </summary>
	WpEntity m_wpOwner;
	//private:
	Space& m_refSpace;
private:
	CoTaskBool CoDelayDelete(const std::chrono::system_clock::duration& dura = std::chrono::seconds(3));
	Position m_Pos;
	std::map<std::string, pro::proxy<Component> > m_mapComponentOnEntityDstroy;

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

