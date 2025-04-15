#pragma once
#include "../../CoRoutine/CoTask.h"
#include "../MyMsgQueue.h"
#include "../单位.h"

class Entity;

class 找目标走过去Component
{
public:
	static void AddComponent(Entity& refEntity);
	static Position 怪物闲逛(const Position& refOld);
	static void 播放前摇动作(Entity& refEntity);
	static void 播放攻击动作(Entity& refEntity);
	static void 播放攻击音效(Entity& refEntity);

	找目标走过去Component(Entity& refEntity);
	float 攻击距离(const Entity& refTarget)const;
	float 攻击距离(const float f目标建筑半边长) const;

	void 顶层大循环(const std::function<bool()> fun可以操作, const std::function<WpEntity()> fun找最近的目标, const std::function<CoTask<std::tuple<bool, bool>>(const Entity& refTarget, WpEntity wpEntity, 找目标走过去Component& ref找目标走过去)> fun操作最近的目标, const std::function<void(WpEntity& wpEntity, bool& ref仇恨目标)> fun处理仇恨目标);

	
	单位::战斗配置 m_战斗配置;
	bool m_b搜索新的目标 = true;
	bool m_b原地坚守 = false;//Hold Position
	using Fun空闲走向目标 = std::function<Position(const Position&)>;
	Fun空闲走向目标 m_fun空闲走向此处;
	void TryCancel(const bool bDestroy = false);
	bool 检查穿墙(const Entity& refEntity);

private:
	CoTaskBool Co顶层(
		const std::function<bool()> fun可以操作,
		const std::function<WpEntity()> fun找最近的目标,
		const std::function<CoTask<std::tuple<bool, bool>>(const Entity& refTarget, WpEntity wpEntity, 找目标走过去Component& ref找目标走过去)> fun操作最近的目标,
		const std::function<void(WpEntity& wpEntity, bool& ref仇恨目标)> fun处理仇恨目标);

	CoTaskBool Co走向警戒范围内的目标然后操作(
		const std::function<bool()> fun可以操作,
		const std::function<WpEntity()> fun找最近的目标,
		const std::function<CoTask<std::tuple<bool, bool>>(const Entity& refTarget, WpEntity wpEntity, 找目标走过去Component& ref找目标走过去)> fun操作最近的目标,
		const std::function<void(WpEntity& wpEntity, bool& ref仇恨目标)> fun处理仇恨目标);

	Entity& m_refEntity;
	CoTaskCancel m_TaskCancel顶层;
	CoTaskCancel m_TaskCancel;

};

