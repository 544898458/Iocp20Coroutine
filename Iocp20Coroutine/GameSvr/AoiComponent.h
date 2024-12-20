#pragma once
#include <unordered_map>
#include "SpEntity.h"
class Space;
class AoiComponent
{
public:
	static void Add(Space& refSpace, Entity& refEntity);
	void OnDestory();
	std::unordered_map<uint64_t, WpEntity> m_map我能看到的;
	std::unordered_map<uint64_t, WpEntity> m_map能看到我的;
	int m_i32视野范围 = 0;//视口设置为屏幕范围，其它比警戒范围大一点
private:
	AoiComponent(Space& refSpace, Entity& refEntity);
	Space& m_refSpace;
	Entity& m_refEntity;
	
};

