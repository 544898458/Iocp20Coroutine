#include "pch.h"
#include "BuildingComponent.h"
#include "Entity.h"
#include "PlayerComponent.h"
#include "PlayerGateSession_Game.h"
#include "临时阻挡Component.h"
#include "../CoRoutine/CoTimer.h"
#include "../IocpNetwork/StrConv.h"
#include "单位.h"

void BuildingComponent::AddComponent(Entity& refThis, PlayerGateSession_Game& refSession, const 建筑单位类型 类型, float f半边长)
{
	refThis.m_spBuilding = std::make_shared<BuildingComponent, PlayerGateSession_Game&, const 建筑单位类型&, Entity&>(refSession, 类型, refThis);
	临时阻挡Component::AddComponent(refThis, f半边长);
}

BuildingComponent::BuildingComponent(PlayerGateSession_Game& refSession, const 建筑单位类型& 类型, Entity& refEntity) :m_类型(类型), m_refEntity(refEntity)
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
			oss << "建造完成";
		else
			oss << "正在建造:" << this->m_n建造进度百分比 << "%";

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
	if (!refEntity.m_spBuilding)
		return 0;

	if (单位::Find建筑单位配置(refEntity.m_spBuilding->m_类型, 配置))
		return 配置.f半边长;

	assert(false);
	return 0;
}