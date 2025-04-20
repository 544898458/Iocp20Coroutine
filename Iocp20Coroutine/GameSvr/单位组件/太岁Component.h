#pragma once

#include "../SpEntity.h"

struct Position;
class 太岁Component
{
public:
	太岁Component(Entity&);
	static void AddComponent(Entity& refEntity);
	void 分裂(const Position& refPos);
	WpEntity m_wp分裂;
private:

	/// <summary>
	/// 自己是建筑，虫巢或者太岁
	/// </summary>
	Entity& m_refEntity;
};

