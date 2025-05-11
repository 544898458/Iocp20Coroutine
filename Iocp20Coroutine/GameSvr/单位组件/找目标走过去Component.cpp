#include "pch.h"
#include "../Entity.h"
#include "../EntitySystem.h"
#include "../Space.h"
#include "../CoRoutine/CoTimer.h"
#include "../AiCo.h"
#include "走Component.h"
#include "采集Component.h"
#include "AttackComponent.h"
#include "BuildingComponent.h"
#include "找目标走过去Component.h"
#include "DefenceComponent.h"
#include "AoiComponent.h"

找目标走过去Component::找目标走过去Component(Entity& refEntity) :m_refEntity(refEntity)
{
	CHECK_RET_VOID(单位::Find战斗配置(refEntity.m_类型, m_战斗配置));
}

void 找目标走过去Component::AddComponent(Entity& refEntity)
{
	refEntity.AddComponentOnDestroy(&Entity::m_up找目标走过去, refEntity);
}

float 找目标走过去Component::攻击距离(const Entity& refTarget) const
{
	const float f目标建筑半边长 = BuildingComponent::建筑半边长(refTarget);
	return 攻击距离(f目标建筑半边长);
}
float 找目标走过去Component::攻击距离(const float f目标建筑半边长) const
{
	CHECK_RET_DEFAULT(m_refEntity.m_up找目标走过去);
	if (m_refEntity.m_wpOwner.expired())
		return BuildingComponent::建筑半边长(m_refEntity) + m_refEntity.m_up找目标走过去->m_战斗配置.f攻击距离 + f目标建筑半边长;//普通战斗单位

	auto spOwner = m_refEntity.m_wpOwner.lock();
	return BuildingComponent::建筑半边长(*spOwner) + m_refEntity.m_up找目标走过去->m_战斗配置.f攻击距离 + f目标建筑半边长;
}

void 找目标走过去Component::顶层大循环(
	const std::function<bool()> fun可以操作,
	const std::function<WpEntity()> fun找最近的目标,
	const std::function<CoTask<std::tuple<bool, bool>>(const Entity& refTarget, WpEntity wpEntity, 找目标走过去Component& ref找目标走过去)> fun操作最近的目标,
	const std::function<void(WpEntity& wpEntity, bool& ref仇恨目标)> fun处理仇恨目标)
{
	m_TaskCancel顶层.TryRun(CoTaskBool(Co顶层(fun可以操作, fun找最近的目标, fun操作最近的目标, fun处理仇恨目标)));
}

CoTaskBool 找目标走过去Component::Co顶层(
	const std::function<bool()> fun可以操作,
	const std::function<WpEntity()> fun找最近的目标,
	const std::function<CoTask<std::tuple<bool, bool>>(const Entity& refTarget, WpEntity wpEntity, 找目标走过去Component& ref找目标走过去)> fun操作最近的目标,
	const std::function<void(WpEntity& wpEntity, bool& ref仇恨目标)> fun处理仇恨目标)
{
	using namespace std;
	//while (!co_await CoTimer::Wait(1000ms, m_funCancel顶层))
	KeepCancel kc(m_TaskCancel顶层.cancel);
	bool b外部取消协程 = false;
	FunCancel funCancel = [&b外部取消协程]()
		{
			b外部取消协程 = true;
		};
	m_TaskCancel顶层.cancel = funCancel;
	while (!co_await CoTimer::WaitNextUpdate(m_TaskCancel顶层.cancel))
	{
		if (b外部取消协程)
			co_return true;

		if (m_refEntity.IsDead())
			co_return false;

		if (!m_refEntity.m_wpOwner.expired() && 房虫 == m_refEntity.m_wpOwner.lock()->m_类型)
			continue;

		if (!fun可以操作())
			continue;

		if (m_b搜索新的目标 && co_await Co走向警戒范围内的目标然后操作(fun可以操作, fun找最近的目标, fun操作最近的目标, fun处理仇恨目标))
		{
			if (b外部取消协程)
				co_return true;

			continue;//这里可能有问题，无法区分是手动停止走路打断还是销毁对象打断
		}
		if (m_refEntity.m_wpOwner.expired() && !m_b原地坚守 && m_refEntity.m_up走 && m_fun空闲走向此处 && !走Component::正在走(m_refEntity) && !采集Component::正在采集(m_refEntity))//打完走向下一个目标
		{
			走Component::Cancel所有包含走路的协程(m_refEntity); //TryCancel();

			auto posTarget = m_fun空闲走向此处(m_refEntity.Pos());
			m_refEntity.m_refSpace.CrowdToolFindNerestPos(posTarget);
			if (m_refEntity.Pos().DistanceLessEqual(posTarget, 3))
			{
				m_fun空闲走向此处 = m_refEntity.m_upPlayerNickName ? nullptr : 怪物闲逛;
				continue;
			}

			走Component::WalkToPos(m_refEntity, posTarget);
			continue;
		}
	}
	co_return true;
}


CoTaskBool 找目标走过去Component::Co走向警戒范围内的目标然后操作(
	const std::function<bool()> fun可以操作,
	const std::function<WpEntity()> fun找最近的目标,
	const std::function<CoTask<std::tuple<bool, bool>>(const Entity& refTarget, WpEntity wpEntity, 找目标走过去Component& ref找目标走过去)> fun操作最近的目标,
	const std::function<void(WpEntity& wpEntity, bool& ref仇恨目标)> fun处理仇恨目标)
{
	CHECK_CO_RET_FALSE(m_refEntity.m_upAoi);
	KeepCancel kc(m_TaskCancel.cancel);
	bool b外部取消协程(false);
	FunCancel funCancel = [&b外部取消协程]()
		{
			b外部取消协程 = true;
		};
	m_TaskCancel.cancel = funCancel;
	while (true)
	{
		if (co_await CoTimer::WaitNextUpdate(m_TaskCancel.cancel))
			co_return true;

		if (!fun可以操作())
			co_return false;

		//CHECK_CO_RET_FALSE(m_refEntity.m_upAoi);
		//if (m_refEntity.m_upAoi->m_map我能看到的.empty())
		//{
		//	m_b搜索新的目标 = false;//AOI内警戒范围内没有目标
		//	co_return false;
		//}

		auto wpEntity = fun找最近的目标();
		if (wpEntity.expired())
		{
			co_return false;
		}

		{
			Entity& refTarget = *wpEntity.lock();
			const auto [bStop, bContinue] = co_await fun操作最近的目标(refTarget, wpEntity, *this);
			if (b外部取消协程)
				co_return true;

			if (bStop)
				co_return true;

			if (bContinue)
				continue;

			if (m_b原地坚守)
				continue;
		}

		bool b仇恨目标 = false;

		fun处理仇恨目标(wpEntity, b仇恨目标);

		if (wpEntity.expired())
		{
			//m_b搜索新的目标 = false;//警戒范围内没有目标
			co_return false;
		}

		Entity& refTarget = *wpEntity.lock();

		if (m_refEntity.m_wpOwner.expired() && (b仇恨目标 || m_refEntity.DistanceLessEqual(refTarget, m_refEntity.警戒距离())) &&
			//!走Component::正在走(m_refEntity) && 
			(!m_refEntity.m_up采集 || m_refEntity.m_up采集->m_TaskCancel.co.Finished())
			)
		{
			走Component::Cancel所有包含走路的协程(m_refEntity); //TryCancel();

			if (m_refEntity.m_up采集)
			{
				m_refEntity.m_up采集->m_TaskCancel.TryCancel();
			}

			if (m_refEntity.m_up走)//炮台没有走组件
			{
				if (co_await m_refEntity.m_up走->WalkToTarget(wpEntity.lock(), m_TaskCancel.cancel, !b仇恨目标, [this](Entity& ref) {return 检查穿墙(ref); }))
					co_return true;
			}
			continue;//可能已走到攻击距离内，再攻击试试
		}

		co_return false;
	}

	co_return false;
}

bool 找目标走过去Component::检查穿墙(const Entity& refEntity)
{
	if (绿色坦克 != m_refEntity.m_类型)
		return true;

	auto pos走向目标点 = refEntity.Pos();
	const auto f建筑半边长 = BuildingComponent::建筑半边长(refEntity);
	//单位::战斗配置 战斗配置;
	if (0 < f建筑半边长)//&& 单位::Find战斗配置(m_refEntity.m_类型, 战斗配置))
	{
		const auto vec直线 = refEntity.Pos() - m_refEntity.Pos();

		if (vec直线.Length() <= f建筑半边长)
			return true;

		const auto vec方向 = vec直线.归一化();
		const auto f走到攻击范围内 = vec直线.Length() - f建筑半边长;
		pos走向目标点 = m_refEntity.Pos() + vec方向 * f走到攻击范围内;
	}

	if (!m_refEntity.m_refSpace.CrowdTool可走直线(m_refEntity.Pos(), pos走向目标点))
		return false;

	return true;
}

void 找目标走过去Component::OnEntityDestroy(const bool bDestroy)
{
	if (bDestroy)
	{
		m_TaskCancel顶层.TryCancel();
	}

	m_TaskCancel.TryCancel();
}


Position 找目标走过去Component::怪物闲逛(const Position& refOld)
{
	auto posTarget = refOld;
	posTarget.x += std::rand() % 11 - 5;//随机走
	posTarget.z += std::rand() % 11 - 5;
	return posTarget;
}

void 找目标走过去Component::播放前摇动作(Entity& refEntity)
{
	CHECK_RET_VOID(refEntity.m_up找目标走过去);
	if (!refEntity.m_up找目标走过去->m_战斗配置.str前摇动作.empty())
		refEntity.BroadcastChangeSkeleAnim(refEntity.m_up找目标走过去->m_战斗配置.str前摇动作);
}


void 找目标走过去Component::播放攻击动作(Entity& refEntity)
{
	CHECK_RET_VOID(refEntity.m_up找目标走过去);
	if (!refEntity.m_up找目标走过去->m_战斗配置.str攻击动作.empty())
		refEntity.BroadcastChangeSkeleAnim(refEntity.m_up找目标走过去->m_战斗配置.str攻击动作);
}


void 找目标走过去Component::播放攻击音效(Entity& refEntity)
{
	//switch (m_类型)
	//{
	//case 兵:
	//	if (!m_refEntity.m_upPlayer)
	//		EntitySystem::Broadcast播放声音(m_refEntity, "TMaFir00");
	//	break;
	//case 近战兵:EntitySystem::Broadcast播放声音(m_refEntity, "Tfrshoot"); break;
	//case 工程车:EntitySystem::Broadcast播放声音(m_refEntity, "TSCMin00"); break;
	//case 三色坦克:EntitySystem::Broadcast播放声音(m_refEntity, "音效/TTaFi200"); break;
	//}

	CHECK_RET_VOID(refEntity.m_up找目标走过去);
	if (!refEntity.m_up找目标走过去->m_战斗配置.str攻击音效.empty())
		EntitySystem::Broadcast播放声音(refEntity, refEntity.m_up找目标走过去->m_战斗配置.str攻击音效);
}