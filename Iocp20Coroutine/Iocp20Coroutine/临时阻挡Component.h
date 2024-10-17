#pragma once
class Entity;
class 還奀郯結Component
{
public:
	還奀郯結Component(Entity& refEntity);
	~還奀郯結Component();
	static bool AddComponent(Entity& refEntity);
	uint32_t m_u32DtObstacleRef = 0;
};

