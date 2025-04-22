#include "pch.h"
#include "AttackComponent.h"
#include "Entity.h"
#include "EntitySystem.h"
#include "PlayerComponent.h"
#include "MonsterComponent.h"
#include "../IocpNetwork/StrConv.h"
#include "GameSvrSession.h"
#include "AiCo.h"
#include "Space.h"
#include "PlayerGateSession_Game.h"
#include "采集Component.h"
#include "走Component.h"
#include "造建筑Component.h"
#include "../CoRoutine/CoTimer.h"
#include "DefenceComponent.h"
#include "BuildingComponent.h"
#include "AoiComponent.h"
#include "临时阻挡Component.h"
#include "飞向目标Component.h"
#include "找目标走过去Component.h"

extern std::unordered_map<int, uint64_t> m_mapEntityId;
AttackComponent& AttackComponent::AddComponent(Entity& refEntity)
{
	找目标走过去Component::AddComponent(refEntity);
	refEntity.AddComponentOnDestroy(&Entity::m_upAttack, new AttackComponent(refEntity));
	return *refEntity.m_upAttack;
}

using namespace std;
AttackComponent::AttackComponent(Entity& refEntity) :
	m_refEntity(refEntity)
{
	CHECK_RET_VOID(m_refEntity.m_up找目标走过去);

	m_refEntity.m_up找目标走过去->顶层大循环(
		[this]()->bool {return this->可以攻击(); },
		[this]()->WpEntity {return Get最近的敌人(); },
		[this](const Entity& refTarget, WpEntity wpEntity, 找目标走过去Component& ref找目标走过去)->CoTask<std::tuple<bool, bool>> {return Co攻击(refTarget, wpEntity, ref找目标走过去); },
		[this](WpEntity& wpEntity, bool& ref仇恨目标)->void{ this->处理仇恨目标(wpEntity, ref仇恨目标); }
	);
}

void AttackComponent::OnEntityDestroy(const bool bDestroy)
{
	if (m_cancelAttack)
	{
		//LOG(INFO) << "调用m_cancel";
		m_cancelAttack();
		m_cancelAttack = nullptr;
	}
}

bool AttackComponent::可以攻击()
{
	if (m_refEntity.m_upBuilding && !m_refEntity.m_upBuilding->已造好())
		return false;

	if (m_refEntity.IsDead())
		return false;

	if (造建筑Component::正在建造(m_refEntity))
		return false;

	if (m_refEntity.m_up走)
	{
		if (!m_refEntity.m_up走->m_coWalk手动控制.Finished() ||
			!m_refEntity.m_up走->m_coWalk进地堡.Finished())
			return false;//表示不允许打断
	}
	return true;
}



#define CHECK_终止攻击目标流程 \
		if (m_refEntity.IsDead())\
			co_return false;\
		if (wpDefencer.expired())\
			break;\
		if (wpDefencer.lock()->IsDead())\
			break;\

CoTaskBool AttackComponent::CoAttack目标(WpEntity wpDefencer, FunCancel& cancel)
{
	KeepCancel kc(cancel);
	//活动单位临时阻挡 _(m_refEntity);
	EntitySystem::恢复休闲动作 _(m_refEntity, {});

	do
	{
		CHECK_终止攻击目标流程;

		m_refEntity.m_eulerAnglesY = CalculateAngle(m_refEntity.Pos(), wpDefencer.lock()->Pos());
		m_refEntity.BroadcastNotifyPos();
		找目标走过去Component::播放前摇动作(m_refEntity);

		CHECK_CO_RET_FALSE(m_refEntity.m_up找目标走过去);
		if (0s < m_refEntity.m_up找目标走过去->m_战斗配置.dura开始播放攻击动作 && co_await CoTimer::Wait(m_refEntity.m_up找目标走过去->m_战斗配置.dura开始播放攻击动作, cancel))
			co_return true;//协程取消

		CHECK_终止攻击目标流程;

		找目标走过去Component::播放攻击动作(m_refEntity);
		const auto u16开始伤害 = EntitySystem::升级后攻击前摇_伤害耗时(m_refEntity);
		if (0 < u16开始伤害 && co_await CoTimer::Wait(std::chrono::milliseconds(u16开始伤害), cancel))
			co_return true;//协程取消

		switch (m_refEntity.m_类型)
		{
		case 炮台:
			m_refEntity.Broadcast<Msg弹丸特效>({ .idEntity = m_refEntity.Id, .idEntityTarget = wpDefencer.lock()->Id,.特效name = StrConv::GbkToUtf8("特效/黄泡泡") });
			break;
		default:; break;
		}

		CHECK_终止攻击目标流程;

		auto& refDefencer = *wpDefencer.lock();
		CHECK_CO_RET_FALSE(m_refEntity.m_up找目标走过去);
		if (!m_refEntity.DistanceLessEqual(refDefencer, m_refEntity.m_up找目标走过去->攻击距离(refDefencer)))
			break;//要执行后摇

		if (!refDefencer.m_upDefence)
			break;//目标打不了

		找目标走过去Component::播放攻击音效(m_refEntity);

		const uint16_t u16升级后的攻击 = EntitySystem::升级后攻击(m_refEntity);
		if (0 < u16升级后的攻击)
			refDefencer.m_upDefence->受伤(u16升级后的攻击, m_refEntity.Id);

		if (绿色坦克 == m_refEntity.m_类型)
		{
			auto wp光刺 = m_refEntity.m_refSpace.造活动单位(std::forward<UpPlayerComponent>(m_refEntity.m_upPlayer), EntitySystem::GetNickName(m_refEntity), m_refEntity.Pos(), 光刺);
			_ASSERT(!wp光刺.expired());
			CHECK_WP_CO_RET_FALSE(wp光刺);
			auto& ref光刺 = *wp光刺.lock();
			CHECK_CO_RET_FALSE(m_refEntity.m_up找目标走过去);
			飞向目标Component::AddComponent(ref光刺, m_refEntity.Pos(), (refDefencer.Pos() - m_refEntity.Pos()).归一化(), m_refEntity.m_up找目标走过去->m_战斗配置.f攻击距离);
			_ASSERT(ref光刺.m_up飞向目标);
		}
	} while (false);

	CHECK_CO_RET_FALSE(m_refEntity.m_up找目标走过去);
	if (co_await CoTimer::Wait(m_refEntity.m_up找目标走过去->m_战斗配置.dura后摇, cancel))//后摇
		co_return true;//协程取消

	if (!m_refEntity.IsDead())
	{
		EntitySystem::BroadcastChangeSkeleAnimIdle(m_refEntity);//播放休闲待机动作
	}

	co_return false;//协程正常退出
}

#define CHECK_终止攻击位置流程 \
		if (m_refEntity.IsDead())\
			co_return false;\

CoTaskBool AttackComponent::CoAttack位置(const Position posTarget, const float f目标建筑半边长, FunCancel& cancel)
{
	KeepCancel kc(cancel);
	//活动单位临时阻挡 _(m_refEntity);

	do
	{
		CHECK_终止攻击位置流程;

		m_refEntity.m_eulerAnglesY = CalculateAngle(m_refEntity.Pos(), posTarget);
		m_refEntity.BroadcastNotifyPos();
		找目标走过去Component::播放前摇动作(m_refEntity);
		EntitySystem::BroadcastEntity描述(m_refEntity, "准备开炮");

		const auto u16开始伤害 = EntitySystem::升级后攻击前摇_伤害耗时(m_refEntity);

		{
			SpEntity spEntity特效 = std::make_shared<Entity, const Position&, Space&, 单位类型, const 单位::单位配置&>(
				posTarget, m_refEntity.m_refSpace, 特效, { "炸点" ,无, "特效/炸点","" });
			m_refEntity.m_refSpace.AddEntity(spEntity特效, 0);
			spEntity特效->BroadcastEnter();
			CHECK_CO_RET_FALSE(m_refEntity.m_up找目标走过去);
			spEntity特效->DelayDelete(m_refEntity.m_up找目标走过去->m_战斗配置.dura开始播放攻击动作 + std::chrono::milliseconds(u16开始伤害));
		}
		CHECK_CO_RET_FALSE(m_refEntity.m_up找目标走过去);
		if (0s < m_refEntity.m_up找目标走过去->m_战斗配置.dura开始播放攻击动作 && co_await CoTimer::Wait(m_refEntity.m_up找目标走过去->m_战斗配置.dura开始播放攻击动作, cancel))
			co_return true;//协程取消

		CHECK_终止攻击位置流程;
		EntitySystem::BroadcastEntity描述(m_refEntity, "");
		找目标走过去Component::播放攻击动作(m_refEntity);
		if (0 < u16开始伤害 && co_await CoTimer::Wait(std::chrono::milliseconds(u16开始伤害), cancel))
			co_return true;//协程取消

		CHECK_终止攻击位置流程;

		CHECK_CO_RET_FALSE(m_refEntity.m_up找目标走过去);
		if (!m_refEntity.Pos().DistanceLessEqual(posTarget, m_refEntity.m_up找目标走过去->攻击距离(f目标建筑半边长)))
			break;//要执行后摇

		找目标走过去Component::播放攻击音效(m_refEntity);
		{
			SpEntity spEntity特效 = std::make_shared<Entity, const Position&, Space&, 单位类型, const 单位::单位配置&>(
				posTarget, m_refEntity.m_refSpace, 特效, { "爆炸溅射" ,无 ,"特效/黄光爆闪","" });
			m_refEntity.m_refSpace.AddEntity(spEntity特效, 0);
			spEntity特效->BroadcastEnter();
			spEntity特效->DelayDelete(1s);
		}
		_ASSERT(m_refEntity.m_upAoi);
		if (m_refEntity.m_upAoi)
		{
			auto& refUpAoi = m_refEntity.m_wpOwner.expired() ? m_refEntity.m_upAoi : m_refEntity.m_wpOwner.lock()->m_upAoi;
			CHECK_NOTNULL_CO_RET_FALSE(refUpAoi);
			for (auto [k, wp] : refUpAoi->m_map我能看到的)
			{
				CHECK_WP_CONTINUE(wp);
				auto& refDefencer = *wp.lock();
				if (!EntitySystem::Is空地能打(m_refEntity.m_类型, refDefencer.m_类型))
					continue;

				if (refDefencer.m_upDefence && refDefencer.Pos().DistanceLessEqual(posTarget, 5))
					refDefencer.m_upDefence->受伤(EntitySystem::升级后攻击(m_refEntity), m_refEntity.Id);
			}
		}

	} while (false);

	if (co_await CoTimer::Wait(800ms, cancel))//后摇
		co_return true;//协程取消

	if (!m_refEntity.IsDead())
	{
		EntitySystem::BroadcastChangeSkeleAnimIdle(m_refEntity);//播放休闲待机动作
	}

	co_return false;//协程正常退出
}

void AttackComponent::处理仇恨目标(WpEntity& wpEntity, bool& ref仇恨目标)
{
	//仇恨列表
	if (m_refEntity.m_upDefence)
	{
		//找对我伤害最大的敌人
		auto& refMap = m_refEntity.m_upDefence->m_map对我伤害;
		while (!refMap.empty())
		{
			auto iterBegin = refMap.begin();
			WpEntity wp = m_refEntity.m_refSpace.GetEntity(iterBegin->first);
			if (wp.expired())
			{
				refMap.erase(iterBegin);
				continue;
			}

			auto& refEntity = *wp.lock();
			if (!m_refEntity.IsEnemy(refEntity))
			{
				refMap.erase(iterBegin);
				continue;
			}
			if (!m_refEntity.DistanceLessEqual(refEntity, 35))
			{
				refMap.erase(iterBegin);
				continue;
			}
			if(!EntitySystem::Is空地能打(m_refEntity.m_类型, refEntity.m_类型))
			{
				refMap.erase(iterBegin);
				continue;
			}
			if (refEntity.IsDead())
			{
				refMap.erase(iterBegin);
				continue;
			}
			wpEntity = wp;
			ref仇恨目标 = true;
			break;
		}
	}
}

CoTask<std::tuple<bool, bool>> AttackComponent::Co攻击(const Entity& refTarget, WpEntity wpEntity, 找目标走过去Component& ref找目标走过去)
{
	const bool b距离友方单位太近 = EntitySystem::距离友方单位太近(m_refEntity);

	if (!m_refEntity.DistanceLessEqual(refTarget, ref找目标走过去.攻击距离(refTarget)) || b距离友方单位太近 || !ref找目标走过去.检查穿墙(refTarget))
		co_return{ false, false };

	走Component::Cancel所有包含走路的协程(m_refEntity); //TryCancel();

	if (m_refEntity.m_类型 == 三色坦克)
	{
		if (co_await CoAttack位置(refTarget.Pos(), BuildingComponent::建筑半边长(refTarget), m_cancelAttack))
			co_return{ true, false };
	}
	else
	{
		if (co_await CoAttack目标(wpEntity, m_cancelAttack))
			co_return{ true, false };
	}

	co_return{ false, true };
}

WpEntity AttackComponent::Get最近的敌人()
{
	return m_refEntity.Get最近的Entity支持地堡中的单位(Entity::敌方, [this](const Entity& ref)->bool
		{
			if (!EntitySystem::Is空地能打(m_refEntity.m_类型, ref.m_类型))
				return false;
			if (!ref.m_upDefence)
				return false;
			return nullptr != ref.m_upDefence;
		});
}