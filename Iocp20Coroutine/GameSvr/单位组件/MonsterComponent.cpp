#include "pch.h"
#include "MonsterComponent.h"
#include "Entity.h"
#include "AiCo.h"
#include "AttackComponent.h"
#include "DefenceComponent.h"
#include "Space.h"
#include "走Component.h"
#include "单位.h"
#include "../EntitySystem.h"

void MonsterComponent::AddComponent(Entity& refEntity)
{
	refEntity.m_spMonster = std::make_shared<MonsterComponent, Entity&>(refEntity);
}

MonsterComponent::MonsterComponent(Entity& refThis)
{
	if (!refThis.m_spAttack)
	{
		LOG(ERROR) << "m_spAttack";
		return;
	}
	//m_coIdle = AiCo::Idle(refThis.shared_from_this(), refThis.m_spAttack->m_cancel);
	//m_coIdle.Run();
}


std::vector<SpEntity> MonsterComponent::AddMonster(Space& refSpace, const 单位类型 类型, const Position& refPos, const int count)
{
	//CHECK_RET_DEFAULT(EntitySystem::Is怪(类型));
	std::vector<SpEntity> vecRet;
	单位::怪配置 怪配置;
	单位::单位配置 单位配置;
	单位::战斗配置 战斗配置;
	CHECK_RET_DEFAULT(单位::Find怪配置(类型, 怪配置));
	CHECK_RET_DEFAULT(单位::Find单位配置(类型, 单位配置));
	for (int i = 0; i < count; ++i)
	{
		SpEntity spEntityMonster = std::make_shared<Entity, const Position&, Space&, const 单位类型, const 单位::单位配置&>(
			refPos, refSpace, std::forward<const 单位类型&&>(类型), 单位配置);
		AttackComponent::AddComponent(*spEntityMonster);
		DefenceComponent::AddComponent(*spEntityMonster, 怪配置.u16初始Hp);
		走Component::AddComponent(*spEntityMonster);
		AddComponent(*spEntityMonster);
		//spEntityMonster->m_f警戒距离 = 配置.战斗.f警戒距离;
		spEntityMonster->m_速度每帧移动距离 = 战斗配置.f每帧移动距离;
		//refSpace.m_mapEntity.insert({ (int64_t)spEntityMonster.get() ,spEntityMonster });
		refSpace.AddEntity(spEntityMonster);
		//LOG(INFO) << "SpawnMonster:" << refSpace.m_mapEntity.size();
		spEntityMonster->BroadcastEnter();
		vecRet.emplace_back(spEntityMonster);
	}
	return vecRet;
}