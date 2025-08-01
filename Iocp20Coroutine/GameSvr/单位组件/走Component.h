#pragma once
#include "SpEntity.h"
#include "../CoRoutine/CoTask.h"

struct Position;
class RecastNavigationCrowd;


class 走Component
{
public:
	走Component(Entity& refEntity);
	void WalkToPos手动控制(const Position& posTarget);
	void WalkToTarget(SpEntity spTarget);
	void WalkToPos(const Position& posTarget);
	void OnEntityDestroy(const bool bDestroy);
	void 走进(WpEntity wpEntity地堡);
	static void AddComponent(Entity& refEntity);
	static void Cancel所有包含走路的协程(Entity& refEntity, const bool b停止攻击 = false);
	static bool 正在走(const Entity& refEntity);
	static bool WalkToTarget(Entity& refThis, SpEntity spTarget);
	static bool WalkToPos(Entity& refThis, const Position& posTarget);

	CoTaskBool WalkToTarget(WpEntity wpEntity, FunCancel& funCancel, const bool b检查警戒距离 = true, const std::function<bool()>& fun可停下 = []() {return true; });
	CoTaskBool WalkToPos(const Position posTarget, FunCancel& funCancel, const float f距离目标小于此距离停下 = 0);
	void 停止延时删除RecastNavigationCrowd();

	static const int INVALID_AGENT_IDX = -1;//0是合法值
	int m_idxCrowdAgent = INVALID_AGENT_IDX;

	CoTaskBool m_coWalk;
	CoTaskBool m_coWalk手动控制;
	CoTaskBool m_coWalk进地堡;
	FunCancel安全 m_cancel;
	std::weak_ptr<RecastNavigationCrowd> m_wpRecastNavigationCrowd;
private:
	CoTaskBool Co走进地堡(WpEntity wpEntity地堡);
	FunCancel m_funCancel延时删除RecastNavigationCrowd;
	Entity& m_refEntity;
};

