#pragma once
class Space;
enum 资源类型;
class 资源Component
{
public:
	static void Add(Space& refSpace, const 资源类型 类型, const Position& refPosition);
	资源Component(const 资源类型 类型) :m_类型(类型) {}
	const 资源类型 m_类型;
};

