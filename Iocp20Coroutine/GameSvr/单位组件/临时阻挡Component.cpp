#include "pch.h"
#include "临时阻挡Component.h"
#include "Entity.h"
#include "Space.h"

临时阻挡Component::临时阻挡Component(Entity& refEntity, float f半边长):m_refSpace(refEntity.m_refSpace), m_refEntity(refEntity)
{
	float arrF[] = { refEntity.Pos().x,0,refEntity.Pos().z };
	uint32_t CrowToolAdd方块阻挡(CrowdToolState & ref, float arrF[], float f半边长);
	m_u32DtObstacleRef = CrowToolAdd方块阻挡(refEntity.m_refSpace.GetCrowdToolState(refEntity.m_类型), arrF, f半边长);
}

临时阻挡Component::~临时阻挡Component()
{
	//if (!m_refSpace.m_spCrowdToolState)
	//{
	//	LOG(ERROR) << "m_spCrowdToolState";
	//	return;
	//}
	OnEntityDestroy(true);
}

void 临时阻挡Component::OnEntityDestroy(const bool bDestroy)
{
	bool CrowToolRemove阻挡(CrowdToolState & ref, const uint32_t u32DtObstacleRef);
	CrowToolRemove阻挡(m_refSpace.GetCrowdToolState(m_refEntity.m_类型), m_u32DtObstacleRef);
}

bool 临时阻挡Component::AddComponent(Entity& refEntity, float f半边长)
{
	if (refEntity.m_up临时阻挡)
	{
		_ASSERT(false);
		return false;
	}
	refEntity.AddComponentOnDestroy(&Entity::m_up临时阻挡, new 临时阻挡Component(refEntity, f半边长));
	return true;
}

活动单位走完路加阻挡::活动单位走完路加阻挡(Entity& refEntity) :m_refEntity(refEntity)
{
	m_refEntity.m_up临时阻挡.reset();
}

活动单位走完路加阻挡::~活动单位走完路加阻挡()
{
	//临时阻挡Component::AddComponent(m_refEntity, 2);
}