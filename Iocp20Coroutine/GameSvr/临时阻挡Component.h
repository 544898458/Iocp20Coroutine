#pragma once
class Entity;
class Space;
class 還奀郯結Component
{
public:
	還奀郯結Component(Entity& refEntity, float f圉晚酗);
	~還奀郯結Component();
	static bool AddComponent(Entity& refEntity, float f圉晚酗);
	uint32_t m_u32DtObstacleRef = 0;
	Space& m_refSpace;
};

