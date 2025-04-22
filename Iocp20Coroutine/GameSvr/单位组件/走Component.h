#pragma once
#include "SpEntity.h"
#include "../CoRoutine/CoTask.h"

struct Position;
class RecastNavigationCrowd;


class ��Component
{
public:
	��Component(Entity& refEntity);
	void WalkToPos�ֶ�����(const Position& posTarget);
	void WalkToTarget(SpEntity spTarget);
	void WalkToPos(const Position& posTarget);
	void OnEntityDestroy(const bool bDestroy);
	void �߽��ر�(WpEntity wpEntity�ر�);
	static void AddComponent(Entity& refEntity);
	static void Cancel���а�����·��Э��(Entity& refEntity, const bool bֹͣ���� = false);
	static bool ������(const Entity& refEntity);
	static bool WalkToTarget(Entity& refThis, SpEntity spTarget);
	static bool WalkToPos(Entity& refThis, const Position& posTarget);

	CoTaskBool WalkToTarget(SpEntity spEntity, FunCancel& funCancel, const bool b��龯����� = true, const std::function<bool(Entity&)>& fun��ͣ�� = [](Entity&) {return true; });
	CoTaskBool WalkToPos(const Position posTarget, FunCancel& funCancel, const float f����Ŀ��С�ڴ˾���ͣ�� = 0);

	static const int INVALID_AGENT_IDX = -1;//0�ǺϷ�ֵ
	int m_idxCrowdAgent = INVALID_AGENT_IDX;

	CoTaskBool m_coWalk;
	CoTaskBool m_coWalk�ֶ�����;
	CoTaskBool m_coWalk���ر�;
	FunCancel m_cancel;
	std::weak_ptr<RecastNavigationCrowd> m_wpRecastNavigationCrowd;
private:
	CoTaskBool Co�߽��ر�(WpEntity wpEntity�ر�);

	Entity& m_refEntity;
};

