#include "pch.h"
#include "SpaceSystem.h"
#include "Entity.h"
#include "Space.h"
#include "单位组件/可进活动单位Component.h"
#include "单位组件/AttackComponent.h"
#include "单位组件/DefenceComponent.h"
#include "单位组件/走Component.h"

std::vector<SpEntity> SpaceSystem::AddMonster(Space& refSpace, const 单位类型 类型, const Position& refPos, const int count, 找目标走过去Component::Fun空闲走向目标 fun空闲走向目标)
{
	std::vector<SpEntity> vecRet;
	单位::怪配置 怪配置;
	单位::单位配置 单位配置;
	单位::战斗配置 战斗配置;
	CHECK_RET_DEFAULT(单位::Find怪配置(类型, 怪配置));
	CHECK_RET_DEFAULT(单位::Find单位配置(类型, 单位配置));
	CHECK_RET_DEFAULT(单位::Find战斗配置(类型, 战斗配置));
	for (int i = 0; i < count; ++i)
	{
		SpEntity spEntityMonster = std::make_shared<Entity, const Position&, Space&, const 单位类型, const 单位::单位配置&>(
			refPos, refSpace, std::forward<const 单位类型&&>(类型), 单位配置);

		if (房虫 == 类型)
			可进活动单位Component::AddComponent(*spEntityMonster);
		else
		{
			AttackComponent::AddComponent(*spEntityMonster);
			CHECK_RET_DEFAULT(spEntityMonster->m_up找目标走过去);
			spEntityMonster->m_up找目标走过去->m_fun空闲走向此处 = fun空闲走向目标;
		}


		DefenceComponent::AddComponent(*spEntityMonster, 怪配置.u16初始Hp);
		走Component::AddComponent(*spEntityMonster);
		//AddComponent(*spEntityMonster);


		refSpace.AddEntity(spEntityMonster);
		//LOG(INFO) << "SpawnMonster:" << refSpace.m_mapEntity.size();
		spEntityMonster->BroadcastEnter();
		vecRet.emplace_back(spEntityMonster);
		//auto [pair, ok] = refSpace.m_mapPlayer[""].m_mapWpEntity.insert({ spEntityMonster->Id, spEntityMonster });//怪物就是昵称为""的玩家
		//CHECK_NOT_RETURN(ok);
	}
	return vecRet;
}