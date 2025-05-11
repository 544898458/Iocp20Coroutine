#include "pch.h"
#include "医疗兵Component.h"
#include "../Entity.h"
#include "../EntitySystem.h"
#include "../Space.h"
#include "../CoRoutine/CoTimer.h"
#include "../AiCo.h"
#include "../枚举/BuffId.h"
#include "../枚举/属性类型.h"
#include "走Component.h"
#include "采集Component.h"
#include "AttackComponent.h"
#include "BuildingComponent.h"
#include "找目标走过去Component.h"
#include "DefenceComponent.h"
#include "数值Component.h"
#include "BuffComponent.h"



医疗兵Component::医疗兵Component(Entity& refEntity) :m_refEntity(refEntity)
{
	CHECK_RET_VOID(m_refEntity.m_up找目标走过去);
	m_refEntity.m_up找目标走过去->顶层大循环(
		[this]()->bool {return this->可以治疗(); },
		[this]()->WpEntity {return Get最近的可治疗友方单位(); },
		[this](const Entity& refTarget, WpEntity wpEntity, 找目标走过去Component& ref找目标走过去)->CoTask<std::tuple<bool, bool>> {return Co治疗(refTarget, wpEntity, ref找目标走过去); },
		[this](WpEntity& wpEntity, bool ref仇恨目标)->void {}
	);

	const int 最大能量 = 20;
	数值Component::Set(refEntity, 属性类型::最大能量, 最大能量);
	数值Component::Set(refEntity, 能量, 最大能量);
	CHECK_RET_VOID(m_refEntity.m_upBuff);
	m_refEntity.m_upBuff->定时改数值(医疗兵自动恢复能量, m_refEntity.Id);
}

void 医疗兵Component::AddComponent(Entity& refEntity)
{
	CHECK_RET_VOID(refEntity.m_upBuff);
	refEntity.m_upBuff->定时改数值(医疗兵自动恢复能量, refEntity.Id);

	找目标走过去Component::AddComponent(refEntity);
	refEntity.AddComponentOnDestroy(&Entity::m_up医疗兵, refEntity);
}


bool 医疗兵Component::可以治疗()
{
	if (m_refEntity.IsDead())
		return false;

	if (数值Component::Get(m_refEntity, 能量) <= 0)
		return false;

	CHECK_RET_FALSE(m_refEntity.m_up走);

	if (!m_refEntity.m_up走->m_coWalk手动控制.Finished() || !m_refEntity.m_up走->m_coWalk进地堡.Finished())
		return false;//表示不允许打断

	return true;
}


CoTask<std::tuple<bool, bool>> 医疗兵Component::Co治疗(const Entity& refTarget, WpEntity wpEntity, 找目标走过去Component& ref找目标走过去)
{
	if (!m_refEntity.DistanceLessEqual(refTarget, ref找目标走过去.攻击距离(refTarget)) || !ref找目标走过去.检查穿墙(refTarget))
		co_return{ false, false };

	走Component::Cancel所有包含走路的协程(m_refEntity); //OnEntityDestroy(const bool bDestroy);

	if (co_await Co治疗目标(wpEntity, m_cancel治疗))
		co_return{ true, false };

	co_return{ false, true };
}

WpEntity 医疗兵Component::Get最近的可治疗友方单位()
{
	return m_refEntity.Get最近的Entity支持地堡中的单位(Entity::友方, [this](const Entity& ref)->bool
		{
			if (!ref.m_upDefence)
				return false;

			if (ref.m_upDefence->已满血())
				return false;

			return true;
		});
}


void 医疗兵Component::OnEntityDestroy(const bool bDestroy)
{
	if (m_cancel治疗)
	{
		//LOG(INFO) << "调用m_cancel";
		m_cancel治疗();
		m_cancel治疗 = nullptr;
	}
}

#define CHECK_终止治疗目标流程 \
		if (m_refEntity.IsDead())\
			co_return false;\
		if (wp目标.expired())\
			break;\
		if (wp目标.lock()->IsDead())\
			break;\

CoTaskBool 医疗兵Component::Co治疗目标(WpEntity wp目标, FunCancel& cancel)
{
	KeepCancel kc(cancel);
	//活动单位临时阻挡 _(m_refEntity);
	EntitySystem::恢复休闲动作 _(m_refEntity, {});

	do
	{
		CHECK_终止治疗目标流程;

		m_refEntity.m_eulerAnglesY = CalculateAngle(m_refEntity.Pos(), wp目标.lock()->Pos());
		m_refEntity.BroadcastNotifyPos();
		//找目标走过去Component::播放前摇动作(m_refEntity);

		CHECK_CO_RET_FALSE(m_refEntity.m_up找目标走过去);
		using namespace std;
		//if (0s < m_refEntity.m_up找目标走过去->m_战斗配置.dura开始播放攻击动作 && co_await CoTimer::Wait(m_refEntity.m_up找目标走过去->m_战斗配置.dura开始播放攻击动作, cancel))
		//	co_return true;//协程取消

		CHECK_终止治疗目标流程;

		找目标走过去Component::播放攻击动作(m_refEntity);

		while (true)
		{
			const auto u16开始伤害 = EntitySystem::升级后攻击前摇_伤害耗时(m_refEntity);
			if (0 < u16开始伤害 && co_await CoTimer::Wait(std::chrono::milliseconds(u16开始伤害), cancel))
				co_return true;//协程取消

			CHECK_终止治疗目标流程;

			auto& ref目标 = *wp目标.lock();

			if (!ref目标.m_upDefence)
			{
				LOG(ERROR) << "!ref目标.m_upDefence";
				break;
			}

			if (ref目标.m_upDefence->已满血())
				break;

			CHECK_CO_RET_FALSE(m_refEntity.m_up找目标走过去);
			if (!m_refEntity.DistanceLessEqual(ref目标, m_refEntity.m_up找目标走过去->攻击距离(ref目标)))
				break;//要执行后摇

			if (!ref目标.m_upDefence)
				break;//目标打不了

			找目标走过去Component::播放攻击音效(m_refEntity);

			const auto 加满生命 = std::min<int>(EntitySystem::升级后攻击(m_refEntity), 数值Component::Get(ref目标, 最大生命) - 数值Component::Get(ref目标, 生命));
			CHECK_CO_RET_FALSE(0 < 加满生命);
			const auto 加生命 = std::min<int>(加满生命, 数值Component::Get(m_refEntity, 能量));
			数值Component::改变(ref目标, 生命, 加生命);
			数值Component::改变(m_refEntity, 能量, -加生命);
		}

		CHECK_终止治疗目标流程;


	} while (false);

	CHECK_CO_RET_FALSE(m_refEntity.m_up找目标走过去);
	//if (co_await CoTimer::Wait(m_refEntity.m_up找目标走过去->m_战斗配置.dura后摇, cancel))//后摇
	//	co_return true;//协程取消

	if (!m_refEntity.IsDead())
	{
		EntitySystem::BroadcastChangeSkeleAnimIdle(m_refEntity);//播放休闲待机动作
	}

	co_return false;//协程正常退出
}