#include "pch.h"
#include "還奀郯結Component.h"
#include "Entity.h"

還奀郯結Component::還奀郯結Component(Entity& refEntity, float f圉晚酗)
{
	float arrF[] = { refEntity.m_Pos.x,0,refEntity.m_Pos.z };
	uint32_t CrowToolAdd源輸郯結(float arrF[], float f圉晚酗);
	m_u32DtObstacleRef = CrowToolAdd源輸郯結(arrF, f圉晚酗);
}

還奀郯結Component::~還奀郯結Component()
{
	bool CrowToolRemove郯結(const uint32_t u32DtObstacleRef);
	CrowToolRemove郯結(m_u32DtObstacleRef);
}

bool 還奀郯結Component::AddComponent(Entity& refEntity, float f圉晚酗)
{
	if (refEntity.m_sp還奀郯結)
	{
		assert(false);
		return false;
	}
	refEntity.m_sp還奀郯結 = std::make_shared<還奀郯結Component, Entity&, float>(refEntity, 2.0f);
	return true;
}
