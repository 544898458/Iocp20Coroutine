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
		LOG(INFO) << "ǰһ���콨��Э�̻�û����";
		return;
	}
	m_coAddMoney = AiCo::AddMoney(refEntity, m_cancel);
	m_coAddMoney.Run();
}

void BuildingComponent::TryCancel(Entity& refEntity)
{
	if (m_cancel)
	{
		//LOG(INFO) << "����m_cancel";
		m_cancel();
		assert(m_coAddMoney.Finished());
	}
	else
	{
		//LOG(INFO) << "m_cancel�ǿյģ�û��Ҫȡ����Э��";
		if (!m_coAddMoney.Finished())
		{
			LOG(ERROR) << "Э��û������ȴ��ǰ�����m_cancel";
			assert(false);
		}
	}

	assert(m_coAddMoney.Finished());
}
