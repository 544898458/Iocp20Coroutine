#pragma once
#include <unordered_map>
#include <unordered_set>
#include "SpEntity.h"
class Space;
struct Position;
class AoiComponent
{
public:
	static void Add(Space& refSpace, Entity& refEntity);
	static std::tuple<int, int, int> 格子(const Position& refPos);
	static std::tuple<int, int, int> 格子(const Entity& refEntity);
	std::unordered_map<uint64_t, WpEntity> 能看到的格子里的Entity() const;
	std::unordered_set<int32_t> 能看到的格子() const;
	std::unordered_set<int32_t> 能看到的格子(const Position& pos) const;
	void OnBeforeChangePos(const Position& posNew);
	void 进入Space();
	void 离开Space();
	void Add看到(Entity& refEntity);
	void 看不到(Entity& refEntity被看);
	void OnDestory();
	std::unordered_map<uint64_t, WpEntity> m_map我能看到的;
	std::unordered_map<uint64_t, WpEntity> m_map能看到我的;
	int m_i32视野范围 = 0;//视口设置为屏幕范围，其它比警戒范围大一点
private:
	AoiComponent(Space& refSpace, Entity& refEntity);
	Space& m_refSpace;
	Entity& m_refEntity;

};

