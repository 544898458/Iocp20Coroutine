#include "stdafx.h"
#include "BuildingComponent.h"
#include "Entity.h"
#include "../CoRoutine/CoRpc.h"
#include "AiCo.h"

void Entity::AddComponentBuilding()
{
	m_spBuilding = std::make_shared<BuildingComponent, Entity&>(*this);
}

BuildingComponent::BuildingComponent(Entity& refEntity)
{
	if (!m_coAddMoney.Finished())
	{
		LOG(INFO) << "前一个造建筑协程还没返回";
		return;
	}
	m_coAddMoney = AiCo::AddMoney(refEntity, m_cancel);
	m_coAddMoney.Run();
}

void BuildingComponent::TryCancel(Entity& refEntity)
{
	if (m_cancel)
	{
		//LOG(INFO) << "调用m_cancel";
		m_cancel();
		assert(m_coAddMoney.Finished());
	}
	else
	{
		//LOG(INFO) << "m_cancel是空的，没有要取消的协程";
		if (!m_coAddMoney.Finished())
		{
			LOG(ERROR) << "协程没结束，却提前清空了m_cancel";
			assert(false);
		}
	}

	assert(m_coAddMoney.Finished());
}
