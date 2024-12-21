#pragma once
#include <unordered_map>
#include <set>
#include "SpEntity.h"
class Space;
struct Position;
class AoiComponent
{
public:
	/// <summary>
	/// 
	/// </summary>
	/// <param name="refSpace"></param>
	/// <param name="refEntity"></param>
	/// <param name="i32视野范围">如果是0就用警戒范围当成视野范围</param>
	static void Add(Space& refSpace, Entity& refEntity, const int32_t i32视野范围);
	static std::tuple<int, int, int> 格子(const Position& refPos);
	static std::tuple<int, int, int> 格子(const Entity& refEntity);
	static int 格子Id(const int32_t i32格子X, const int32_t i32格子Z);
	std::unordered_map<int, std::unordered_map<uint64_t, WpEntity>> 能看到的格子里的Entity() const;
	std::set<int32_t> 能看到的格子() const;
	std::set<int32_t> 能看到的格子(const Position& pos) const;
	std::vector<int32_t> 能看到的格子Vec() const;
	void 能看到这一格的人都看到我();
	void 能看到这一格的人都看不到我();
	void 看到这些格子(const std::vector<int32_t>& vec新增看到的格子Id);
	void 看不到这些格子(const std::vector<int32_t>& vec删除不再看到的老格子Id);
	void OnBeforeChangePos(const Position& posNew);
	void 进入Space();
	void 离开Space();
	void 看到(Entity& refEntity被看);
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

