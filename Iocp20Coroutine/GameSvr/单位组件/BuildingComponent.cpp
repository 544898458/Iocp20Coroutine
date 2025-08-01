#include "pch.h"
#include "BuildingComponent.h"
#include "Entity.h"
#include "EntitySystem.h"
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
	m_n建造进度帧 = std::numeric_limits<int>::max();
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

	单位::建筑单位配置 配置;
	单位::制造配置 制造配置;
	CHECK_CO_RET_FALSE(单位::Find建筑单位配置(m_refEntity.m_类型, 配置));
	CHECK_CO_RET_FALSE(单位::Find制造配置(m_refEntity.m_类型, 制造配置));

	const auto u16总耗时帧 = 制造配置.u16耗时帧;
	static_assert(MAX建造十分比 > 0, "MAX建造十分比 <= 0");
	const auto u16每次等帧数 = u16总耗时帧 / MAX建造十分比;
	m_refEntity.BroadcastChangeSkeleAnim(配置.建造.str名字或索引, false, 配置.建造.f播放速度, 配置.建造.f起始时刻秒, 配置.建造.f结束时刻秒);

	this->m_n建造进度帧 = 0;
	while (m_n建造进度帧 < u16总耗时帧)
	{
		const auto u16等待帧数 = std::min(u16每次等帧数, u16总耗时帧 - m_n建造进度帧);
		if (co_await CoTimer::WaitUpdateCount(u16等待帧数, cancel))
			co_return 0;

		this->m_n建造进度帧 += u16每次等帧数;

		std::ostringstream oss;
		if (u16总耗时帧 <= m_n建造进度帧)
			oss << "变异完成";
		else
			oss << "正在变异:" << m_n建造进度帧 << "/" << u16总耗时帧;

		this->m_refEntity.Broadcast<MsgEntity描述>({ .idEntity = this->m_refEntity.Id, .str描述 = StrConv::GbkToUtf8(oss.str()) });
	}

	EntitySystem::BroadcastChangeSkeleAnimIdle(m_refEntity);

	co_return 0;
}

bool BuildingComponent::已造好() const
{
	单位::制造配置 制造配置;	
	CHECK_RET_FALSE(单位::Find制造配置(m_refEntity.m_类型, 制造配置));
	return m_n建造进度帧 >= 制造配置.u16耗时帧;
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