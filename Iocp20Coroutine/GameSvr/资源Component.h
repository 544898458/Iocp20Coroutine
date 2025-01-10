#pragma once
class Space;
enum 单位类型;
struct Position;
class 资源Component
{
public:
	static void Add(Space& refSpace, const 单位类型 类型, const Position& refPosition);
	资源Component(const 单位类型 类型) :m_类型(类型) {}
	const 单位类型 m_类型;
	int m_可采集数量 = 200;
};

