#pragma once
class Entity;
class ��ʱ�赲Component
{
public:
	��ʱ�赲Component(Entity& refEntity);
	~��ʱ�赲Component();
	static bool AddComponent(Entity& refEntity);
	uint32_t m_u32DtObstacleRef = 0;
};

