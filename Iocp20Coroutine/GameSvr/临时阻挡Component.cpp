#include "pch.h"
#include "��ʱ�赲Component.h"
#include "Entity.h"

��ʱ�赲Component::��ʱ�赲Component(Entity& refEntity, float f��߳�)
{
	float arrF[] = { refEntity.m_Pos.x,0,refEntity.m_Pos.z };
	uint32_t CrowToolAdd�����赲(float arrF[], float f��߳�);
	m_u32DtObstacleRef = CrowToolAdd�����赲(arrF, f��߳�);
}

��ʱ�赲Component::~��ʱ�赲Component()
{
	bool CrowToolRemove�赲(const uint32_t u32DtObstacleRef);
	CrowToolRemove�赲(m_u32DtObstacleRef);
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
