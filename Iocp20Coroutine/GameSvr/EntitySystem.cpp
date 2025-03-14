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
	refEntity.BroadcastChangeSkeleAnim(炮台 == refEntity.m_类型 ? "平常状态" : "idle");
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

	CHECK_RET_FALSE(refEntity.m_upAoi);
	//_ASSERT(refEntity.m_upAoi);
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

bool EntitySystem::Is可进地堡(const 单位类型 类型)
{
	if (Is建筑(类型))
		return false;

	switch (类型)
	{
	case 三色坦克:
	case 飞机:
		return false;
	default:
		return true;
	}
}

bool EntitySystem::Is资源(const 单位类型 类型)
{
	return 资源Min非法 < 类型 && 类型 < 资源Max非法;
}

bool EntitySystem::Is活动单位(const 单位类型 类型)
{
	return 活动单位Min非法 < 类型 && 类型 < 活动单位Max非法;
}
bool EntitySystem::Is怪(const 单位类型 类型)
{
	return 怪Min非法 < 类型 && 类型 < 怪Max非法;
}

bool EntitySystem::Is单位类型(const WpEntity& wp, const 单位类型 类型)
{
	if (wp.expired())
		return false;

	return wp.lock()->m_类型 == 类型;
}


bool EntitySystem::Is空地能打(const 单位类型 攻, const 单位类型 防)
{
	if (防 != 飞机)
		return true;

	switch (攻)
	{
	case 兵:
	case 炮台:
	case 飞机:
		return true;
	default:
		return false;
	}
}