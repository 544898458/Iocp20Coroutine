#pragma once
class Entity;
class Space;
class ��ʱ�赲Component
{
public:
	��ʱ�赲Component(Entity& refEntity, float f��߳�);
	~��ʱ�赲Component();
	static bool AddComponent(Entity& refEntity, float f��߳�);
	uint32_t m_u32DtObstacleRef = 0;
	Space& m_refSpace;
};

