#include "pch.h"
#include "BuildingComponent.h"
#include "Entity.h"
#include "PlayerComponent.h"
#include "PlayerGateSession_Game.h"
#include "临时阻挡Component.h"
#include "../CoRoutine/CoTimer.h"
#include "../IocpNetwork/StrConv.h"
#include "单位.h"

void BuildingComponent::AddComponent(Entity& refThis, float f半边长)
{
	refThis.AddComponentOnDestroy(&Entity::m_upBuilding, refThis);
	if (苔蔓 != refThis.m_类型)
		临时阻挡Component::AddComponent(refThis, f半边长);
}

void BuildingComponent::StartCo建造过程()
{
	Co建造过程(m_cancel建造).RunNew();
}

void BuildingComponent::直接造好()
{
	m_n建造进度百分比 = MAX建造百分比;
}

void BuildingComponent::OnEntityDestroy(const bool bDestroy)
{
	m_cancel建造.TryCancel();
}

BuildingComponent::BuildingComponent(Entity& refEntity) :m_refEntity(refEntity), m_cancel建造("m_cancel建造")
{
	//if (!m_coAddMoney.Finished())
	//{
	//	LOG(INFO) << "前一个造建筑协程还没返回";
	//	return;
	//}
	//Co建造过程(m_cancel建造).RunNew();
}

/// <summary>
/// 不需要占用工程车的建造（虫族/异虫 和 神族/星灵 的建筑制造都是自动的）
/// </summary>
/// <param name="cancel"></param>
/// <returns></returns>
CoTaskBool BuildingComponent::Co建造过程(FunCancel& cancel)
{
	KeepCancel kc(cancel);

	while (MAX建造百分比 > this->m_n建造进度百分比)
	{
		if (co_await CoTimer::WaitNextUpdate(cancel))
			co_return 0;

		++this->m_n建造进度百分比;

		std::ostringstream oss;
		if (MAX建造百分比 <= this->m_n建造进度百分比)
			oss << "变异完成";
		else
			oss << "正在变异:" << this->m_n建造进度百分比 << "%";

		this->m_refEntity.Broadcast<MsgEntity描述>({ .idEntity = this->m_refEntity.Id, .str描述 = StrConv::GbkToUtf8(oss.str()) });
	}

	co_return 0;
}

bool BuildingComponent::已造好() const
{
	return m_n建造进度百分比 >= MAX建造百分比;

}

float BuildingComponent::建筑半边长(const Entity& refEntity)
{
	单位::建筑单位配置 配置;
	if (!refEntity.m_upBuilding)
		return 0;

	if (单位::Find建筑单位配置(refEntity.m_类型, 配置))
		return 配置.f半边长;

	_ASSERT(false);
	return 0;
}