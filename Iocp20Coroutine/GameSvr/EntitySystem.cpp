#include "pch.h"
#include "Entity.h"
#include "EntitySystem.h"
#include "MyMsgQueue.h"
#include "../IocpNetwork/StrConv.h"
#include "Space.h"
#include "单位组件/走Component.h"
#include "单位组件/AttackComponent.h"
#include "单位组件/PlayerNickNameComponent.h"

void EntitySystem::BroadcastEntity描述(Entity& refEntity, const std::string& refStrGbk)
{
	refEntity.Broadcast<MsgEntity描述>({ .idEntity = refEntity.Id, .str描述 = StrConv::GbkToUtf8(refStrGbk) });
}

void EntitySystem::BroadcastChangeSkeleAnimIdle(Entity& refEntity)
{
	refEntity.BroadcastChangeSkeleAnim("idle");
}
void EntitySystem::BroadcastChangeSkeleAnim采集(Entity& refEntity)
{
	if (refEntity.m_类型 == 工蜂)
		refEntity.BroadcastChangeSkeleAnim("采集");
}

void EntitySystem::Broadcast播放声音(Entity& refEntity, const std::string& refStr声音, const std::string& str文本)
{
	refEntity.Broadcast<Msg播放声音>({ .str声音 = StrConv::GbkToUtf8(refStr声音), .str文本 = StrConv::GbkToUtf8(str文本) });
}

bool EntitySystem::Is视口(const Entity& refEntity)
{
	if (refEntity.m_spDefence)
		return false;

	if (!refEntity.m_spPlayer)
		return false;

	CHECK_CO_RET_FALSE(refEntity.m_upAoi);
	//assert(refEntity.m_upAoi);
	//if (!refEntity.m_upAoi)
		//return false;

	return true;
}

bool EntitySystem::距离友方单位太近(Entity& refEntity)
{
	const auto wp最近的正在攻击的友方单位 = refEntity.m_refSpace.Get最近的Entity(refEntity, Space::友方,
		[](const Entity& ref)->bool
		{
			if (nullptr == ref.m_spDefence)
				return false;

			if (!ref.m_spAttack)
				return false;

			return (bool)ref.m_spAttack->m_cancelAttack;
		});
	bool b距离友方单位太近 = false;
	if (!wp最近的正在攻击的友方单位.expired())
	{
		b距离友方单位太近 = refEntity.DistanceLessEqual(*wp最近的正在攻击的友方单位.lock(), 3);
	}
	return b距离友方单位太近;
}

const std::string EntitySystem::GetNickName(Entity& refEntity)
{
	if (refEntity.m_spPlayerNickName)
		return refEntity.m_spPlayerNickName->m_strNickName;

	return {};
}

bool EntitySystem::Is建筑(const 单位类型 类型)
{
	return 建筑Min非法 < 类型 && 类型 < 建筑Max非法;
}