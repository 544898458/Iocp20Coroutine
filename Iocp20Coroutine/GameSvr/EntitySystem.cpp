#include "pch.h"
#include "Entity.h"
#include "EntitySystem.h"
#include "MyMsgQueue.h"
#include "../IocpNetwork/StrConv.h"
#include "Space.h"
#include "枚举/BuffId.h"
#include "枚举/属性类型.h"
#include "单位组件/走Component.h"
#include "单位组件/AttackComponent.h"
#include "单位组件/PlayerNickNameComponent.h"
#include "单位组件/找目标走过去Component.h"
#include "单位组件/医疗兵Component.h"
#include "单位组件/苔蔓Component.h"
#include "单位组件/BuffComponent.h"
#include "单位组件/PlayerComponent.h"

void EntitySystem::BroadcastEntity描述(Entity& refEntity, const std::string& refStrGbk)
{
	refEntity.Broadcast<MsgEntity描述>({ .idEntity = refEntity.Id, .str描述 = StrConv::GbkToUtf8(refStrGbk) });
}

void EntitySystem::BroadcastChangeSkeleAnimIdle(Entity& refEntity)
{
	refEntity.BroadcastChangeSkeleAnim(refEntity.m_配置.空闲.str名字或索引, true, refEntity.m_配置.空闲.f播放速度, refEntity.m_配置.空闲.f起始时刻秒, refEntity.m_配置.空闲.f结束时刻秒);
}
void EntitySystem::BroadcastChangeSkeleAnim采集(Entity& refEntity)
{
	if (refEntity.m_类型 == 工虫)
		refEntity.BroadcastChangeSkeleAnim("采集");
}

void EntitySystem::Broadcast播放声音(Entity& refEntity, const std::string& refStr声音, const std::string& str文本)
{
	if (refStr声音.empty())
		return;

	refEntity.Broadcast<Msg播放声音>({ .str声音 = StrConv::GbkToUtf8(refStr声音), .str文本 = StrConv::GbkToUtf8(str文本) });
}

bool EntitySystem::Is视口(const Entity& refEntity)
{
	return Is视口(refEntity.m_类型);
}

bool EntitySystem::Is光刺(const Entity& refEntity)
{
	return Is光刺(refEntity.m_类型);
}
bool EntitySystem::Is活动单位(const Entity& refEntity)
{
	return Is活动单位(refEntity.m_类型);
}
bool EntitySystem::Is活动单位建筑怪(const Entity& refEntity)
{
	return Is活动单位建筑怪(refEntity.m_类型);
}
bool EntitySystem::距离友方单位太近(Entity& refEntity)
{
	const auto wp最近的正在攻击的友方单位 = refEntity.Get最近的Entity(Entity::友方,
		[](const Entity& ref)->bool
	{
		if (nullptr == ref.m_upDefence)
			return false;

		if (!ref.m_upAttack)
			return false;

		return (bool)ref.m_upAttack->m_cancelAttack;
	});
	bool b距离友方单位太近 = false;
	if (!wp最近的正在攻击的友方单位.expired())
	{
		b距离友方单位太近 = refEntity.DistanceLessEqual(*wp最近的正在攻击的友方单位.lock(), 3);
	}
	return b距离友方单位太近;
}

const std::string EntitySystem::GetNickName(const Entity& refEntity)
{
	if (refEntity.m_upPlayerNickName)
		return refEntity.m_upPlayerNickName->m_strNickName;

	return {};
}

bool EntitySystem::Is建筑(const 单位类型 类型)
{
	return 建筑Min非法 < 类型 && 类型 < 建筑Max非法;
}

bool EntitySystem::Is建筑(const Entity& refEntity)
{
	return Is建筑(refEntity.m_类型);
}

bool EntitySystem::Is可进地堡或房虫(const 单位类型 类型)
{
	if (Is建筑(类型))
		return false;

	switch (类型)
	{
	case 近战兵:
	case 近战虫:
	case 枪兵:
	case 枪虫:
	case 工程车:
	case 工虫:
	case 幼虫:
	case 医疗兵:
	case 防空兵:
		return true;
	default:
		return false;
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

bool EntitySystem::Is视口(const 单位类型 类型)
{
	return 视口 == 类型;
}
bool EntitySystem::Is光刺(const 单位类型 类型)
{
	return 光刺 == 类型;
}

bool EntitySystem::Is活动单位建筑怪(const 单位类型 类型)
{
	return Is活动单位(类型) || Is建筑(类型) || Is怪(类型);

}

bool EntitySystem::Is单位类型(const WpEntity& wp, const 单位类型 类型)
{
	if (wp.expired())
		return false;

	return wp.lock()->m_类型 == 类型;
}


bool EntitySystem::Is空地能打(const 单位类型 攻, const 单位类型 防)
{
	if (!EntitySystem::Is活动单位建筑怪(防))
		return false;

	单位::战斗配置 攻配置, 防配置;
	CHECK_RET_FALSE(单位::Find战斗配置(攻, 攻配置));
	CHECK_RET_FALSE(单位::Find战斗配置(防, 防配置));

	if (防配置.b空中)
		return 攻配置.b可打空中;
	else
		return 攻配置.b可打地面;
}

EntitySystem::恢复休闲动作::恢复休闲动作(Entity& refEntity, const 单位::动作& ref动作) :m_refEntity(refEntity)
{
	if (!ref动作.str名字或索引.empty())
		m_refEntity.BroadcastChangeSkeleAnim(ref动作.str名字或索引, true, ref动作.f播放速度, ref动作.f起始时刻秒, ref动作.f结束时刻秒);
}

EntitySystem::恢复休闲动作::~恢复休闲动作()
{
	if (!m_refEntity.IsDead())
		BroadcastChangeSkeleAnimIdle(m_refEntity);
}

template<typename T_成员>
T_成员 EntitySystem::升级后属性(const Entity& refEntity, const 属性类型 属性, T_成员 单位::战斗配置::* p成员)
{
	uint16_t u16属性等级(0);
	if (refEntity.m_upPlayerNickName)
	{
		const auto& spacePlayer = refEntity.m_refSpace.GetSpacePlayer(refEntity);
		u16属性等级 = spacePlayer.单位属性等级(refEntity.m_类型, 属性);
	}

	return 单位::单位升级后属性(refEntity.m_类型, 属性, u16属性等级, p成员);
}

uint16_t EntitySystem::升级后攻击(Entity& refEntity)
{
	//uint16_t u16攻击等级(0);
	//if (refEntity.m_upPlayerNickName)
	//{
	//	const auto& spacePlayer = refEntity.m_refSpace.GetSpacePlayer(refEntity);
	//	u16攻击等级 = spacePlayer.单位属性等级(refEntity.m_类型, 攻击);
	//}

	//return 单位::单位攻击(refEntity.m_类型, u16攻击等级);
	return 升级后属性(refEntity, 攻击, &单位::战斗配置::u16攻击);
}

uint16_t EntitySystem::升级后防御(Entity& refEntity)
{
	return 升级后属性(refEntity, 防御, &单位::战斗配置::u16防御);
}

uint16_t EntitySystem::升级后攻击前摇_伤害耗时(Entity& refEntity)
{
	return 升级后属性(refEntity, 攻击前摇_伤害耗时, &单位::战斗配置::u16开始伤害);
}

float EntitySystem::升级后速度每帧移动距离(Entity& refEntity)
{
	float f速度Buff加数值 = 0;
	if (refEntity.m_upBuff)
		f速度Buff加数值 = refEntity.m_upBuff->属性(移动速度);

	return f速度Buff加数值 + 升级后属性(refEntity, 移动速度, &单位::战斗配置::f每帧移动距离);
}

float EntitySystem::升级后速度每秒移动距离(Entity& refEntity)
{
	return 升级后速度每帧移动距离(refEntity) * 10;
}

void EntitySystem::停止攻击和治疗(Entity& refEntity)
{
	if (refEntity.m_upAttack)
		refEntity.m_upAttack->OnEntityDestroy();

	if (refEntity.m_up找目标走过去)
		refEntity.m_up找目标走过去->OnEntityDestroy();

	if (refEntity.m_up医疗兵)
		refEntity.m_up医疗兵->OnEntityDestroy(false);
}

void EntitySystem::BroadcastEntity苔蔓半径(Entity& refEntity)
{
	CHECK_RET_VOID(refEntity.m_up苔蔓);
	refEntity.Broadcast(Msg苔蔓半径(refEntity));
}

bool EntitySystem::判断前置单位存在(Entity& refEntity, const 单位类型 类型)
{
	//读取制造配置
	单位::制造配置 制造;
	if (!单位::Find制造配置(类型, 制造))
	{
		_ASSERT(false);
		return true;
	}

	//判断前置单位是否存在
	if (单位类型::单位类型_Invalid_0 != 制造.前置单位 && 0 == refEntity.m_refSpace.Get玩家单位数(EntitySystem::GetNickName(refEntity), 制造.前置单位))
	{
		//读取前置单位配置
		单位::单位配置 前置单位;
		if (!单位::Find单位配置(制造.前置单位, 前置单位))
		{
			_ASSERT(false);
			return true;
		}

		PlayerComponent::播放声音Buzz(refEntity, "缺少 " + 前置单位.strName);
		return false;
	}

	return true;
}