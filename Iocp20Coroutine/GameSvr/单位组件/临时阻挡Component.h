#pragma once
class Entity;
class Space;
class 临时阻挡Component
{
public:
	临时阻挡Component(Entity& refEntity, float f半边长);
	~临时阻挡Component();
	static bool AddComponent(Entity& refEntity, float f半边长);
	uint32_t m_u32DtObstacleRef = 0;
	Space& m_refSpace;
	Entity& m_refEntity;
};

class 活动单位走完路加阻挡 final
{
public:
	活动单位走完路加阻挡(Entity &refEntity);
	~活动单位走完路加阻挡();
	Entity& m_refEntity;
};