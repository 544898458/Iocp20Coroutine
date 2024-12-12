#include "pch.h"
#include "��ʱ�赲Component.h"
#include "Entity.h"
#include "Space.h"

��ʱ�赲Component::��ʱ�赲Component(Entity& refEntity, float f��߳�):m_refSpace(refEntity.m_refSpace)
{
	float arrF[] = { refEntity.m_Pos.x,0,refEntity.m_Pos.z };
	uint32_t CrowToolAdd�����赲(CrowdToolState & ref, float arrF[], float f��߳�);
	if (!refEntity.m_refSpace.m_spCrowdToolState)
	{
		LOG(ERROR) << "m_spCrowdToolState";
		return;
	}
	m_u32DtObstacleRef = CrowToolAdd�����赲(*refEntity.m_refSpace.m_spCrowdToolState, arrF, f��߳�);
}

��ʱ�赲Component::~��ʱ�赲Component()
{
	if (!m_refSpace.m_spCrowdToolState)
	{
		LOG(ERROR) << "m_spCrowdToolState";
		return;
	}

	bool CrowToolRemove�赲(CrowdToolState & ref, const uint32_t u32DtObstacleRef);
	CrowToolRemove�赲(*m_refSpace.m_spCrowdToolState, m_u32DtObstacleRef);
}

bool ��ʱ�赲Component::AddComponent(Entity& refEntity, float f��߳�)
{
	if (refEntity.m_sp��ʱ�赲)
	{
		assert(false);
		return false;
	}
	refEntity.m_sp��ʱ�赲 = std::make_shared<��ʱ�赲Component, Entity&, float>(refEntity, 2.0f);
	return true;
}
