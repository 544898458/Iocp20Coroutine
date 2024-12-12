#include "pch.h"
#include "還奀郯結Component.h"
#include "Entity.h"
#include "Space.h"

還奀郯結Component::還奀郯結Component(Entity& refEntity, float f圉晚酗):m_refSpace(refEntity.m_refSpace)
{
	float arrF[] = { refEntity.m_Pos.x,0,refEntity.m_Pos.z };
	uint32_t CrowToolAdd源輸郯結(CrowdToolState & ref, float arrF[], float f圉晚酗);
	if (!refEntity.m_refSpace.m_spCrowdToolState)
	{
		LOG(ERROR) << "m_spCrowdToolState";
		return;
	}
	m_u32DtObstacleRef = CrowToolAdd源輸郯結(*refEntity.m_refSpace.m_spCrowdToolState, arrF, f圉晚酗);
}

還奀郯結Component::~還奀郯結Component()
{
	if (!m_refSpace.m_spCrowdToolState)
	{
		LOG(ERROR) << "m_spCrowdToolState";
		return;
	}

	bool CrowToolRemove郯結(CrowdToolState & ref, const uint32_t u32DtObstacleRef);
	CrowToolRemove郯結(*m_refSpace.m_spCrowdToolState, m_u32DtObstacleRef);
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
