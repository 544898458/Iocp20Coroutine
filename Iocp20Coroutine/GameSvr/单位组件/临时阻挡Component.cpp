#include "pch.h"
#include "��ʱ�赲Component.h"
#include "Entity.h"
#include "Space.h"

��ʱ�赲Component::��ʱ�赲Component(Entity& refEntity, float f��߳�):m_refSpace(refEntity.m_refSpace), m_refEntity(refEntity)
{
	float arrF[] = { refEntity.Pos().x,0,refEntity.Pos().z };
	uint32_t CrowToolAdd�����赲(CrowdToolState & ref, float arrF[], float f��߳�);
	m_u32DtObstacleRef = CrowToolAdd�����赲(refEntity.m_refSpace.GetCrowdToolState(refEntity.m_����), arrF, f��߳�);
}

��ʱ�赲Component::~��ʱ�赲Component()
{
	//if (!m_refSpace.m_spCrowdToolState)
	//{
	//	LOG(ERROR) << "m_spCrowdToolState";
	//	return;
	//}

	bool CrowToolRemove�赲(CrowdToolState & ref, const uint32_t u32DtObstacleRef);
	CrowToolRemove�赲(m_refSpace.GetCrowdToolState(m_refEntity.m_����), m_u32DtObstacleRef);
}

bool ��ʱ�赲Component::AddComponent(Entity& refEntity, float f��߳�)
{
	if (refEntity.m_sp��ʱ�赲)
	{
		_ASSERT(false);
		return false;
	}
	refEntity.m_sp��ʱ�赲 = std::make_shared<��ʱ�赲Component, Entity&, const float&>(refEntity, f��߳�);
	return true;
}

���λ����·���赲::���λ����·���赲(Entity& refEntity) :m_refEntity(refEntity)
{
	m_refEntity.m_sp��ʱ�赲.reset();
}

���λ����·���赲::~���λ����·���赲()
{
	//��ʱ�赲Component::AddComponent(m_refEntity, 2);
}