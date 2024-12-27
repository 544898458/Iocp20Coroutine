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

extern std::unordered_map<int, uint64_t> m_mapEntityId;
void AttackComponent::AddComponent(Entity& refEntity, const 活动单位类型 类型, const 单位::战斗配置& 配置)
{
	CHECK_VOID(!refEntity.m_spAttack);
	refEntity.m_spAttack = std::make_shared<AttackComponent, Entity&, const 活动单位类型, const 单位::战斗配置&>(
		refEntity, std::forward<const 活动单位类型&&>(类型), 配置);
	//refEntity.m_spAttack->m_f攻击距离 = f攻击距离;
	//refEntity.m_spAttack->m_f警戒距离 = f警戒距离;
	//refEntity.m_spAttack->m_f伤害 = f伤害;
	//float arrF[] = { refEntity.Pos().x,0,refEntity.Pos().z};
	//int CrowToolAddAgent(float arrF[]);
	//refEntity.m_spAttack->m_idxCrowdAgent = CrowToolAddAgent(arrF);
	//m_mapEntityId[refEntity.m_spAttack->m_idxCrowdAgent] = refEntity.Id;
	refEntity.m_spAttack->Co顶层().RunNew();
}

float AttackComponent::攻击距离(const Entity& refTarget) const
{
	const float f目标建筑半边长 = BuildingComponent::建筑半边长(refTarget);
	return 攻击距离(f目标建筑半边长);
}
float AttackComponent::攻击距离(const float f目标建筑半边长) const
{
	if (m_refEntity.m_wpOwner.expired())
		return m_战斗配置.f攻击距离 + f目标建筑半边长;//普通战斗单位

	auto spOwner = m_refEntity.m_wpOwner.lock();
	return m_战斗配置.f攻击距离 + f目标建筑半边长 + BuildingComponent::建筑半边长(*spOwner);
}
Position 怪物闲逛(const Position& refOld)
{
	auto posTarget = refOld;
	posTarget.x += std::rand() % 11 - 5;//随机走
	posTarget.z += std::rand() % 11 - 5;
	return posTarget;
}
using namespace std;
AttackComponent::AttackComponent(Entity& refEntity, const 活动单位类型 类型, const 单位::战斗配置& 配置) :
	m_refEntity(refEntity),
	m_类型(类型),
	m_fun空闲走向此处(怪物闲逛),
	m_战斗配置(配置)
{
}

void AttackComponent::TryCancel(const bool bDestroy)
{
	if (m_cancelAttack)
	{
		//LOG(INFO) << "调用m_cancel";
		m_cancelAttack();
	}

	m_TaskCancel.TryCancel();

	if (bDestroy && m_funCancel顶层)
		m_funCancel顶层();
}

CoTaskBool AttackComponent::Co顶层()
{
	using namespace std;
	//while (!co_await CoTimer::Wait(1000ms, m_funCancel顶层))
	while (!co_await CoTimer::WaitNextUpdate(m_funCancel顶层))
	{
		if (m_refEntity.IsDead())
			co_return false;


		if (m_b搜索新的目标 && co_await Co走向警戒范围内的目标然后攻击(m_TaskCancel.cancel))
			continue;

		if (!m_refEntity.m_spPlayer && !走Component::正在走(m_refEntity))//怪随机走
		{
			走Component::Cancel所有包含走路的协程(m_refEntity); //TryCancel();

			auto posTarget = m_fun空闲走向此处(m_refEntity.Pos());
			m_refEntity.m_refSpace.CrowdToolFindNerestPos(posTarget);
			走Component::WalkToPos(m_refEntity, posTarget);
			continue;
		}
	}
	co_return true;
}

void AttackComponent::Update()
{
}

bool AttackComponent::可以攻击()
{
	if (m_refEntity.m_sp走 && !m_refEntity.m_sp走->m_coWalk手动控制.Finished())
		return false;//表示不允许打断

	if (m_refEntity.IsDead())
		return false;

	if (造建筑Component::正在建造(m_refEntity))
		return false;

	if (m_refEntity.m_sp走)
	{
		if (!m_refEntity.m_sp走->m_coWalk手动控制.Finished() ||
			!m_refEntity.m_sp走->m_coWalk进地堡.Finished())
			return false;//表示不允许打断
	}
	return true;
}

CoTaskBool AttackComponent::Co走向警戒范围内的目标然后攻击(FunCancel& funCancel)
{
	KeepCancel kc(funCancel);
	while (true)
	{
		if (!可以攻击())
			co_return false;

		const auto wpEntity = m_refEntity.m_refSpace.Get最近的Entity支持地堡中的单位(m_refEntity, true, [](const Entity& ref)->bool {return nullptr != ref.m_spDefence; });
		if (wpEntity.expired())
		{
			m_b搜索新的目标 = false;//警戒范围内没有目标
			co_return false;
		}

		const auto wp最近的友方单位 = m_refEntity.m_refSpace.Get最近的Entity(m_refEntity, false, [](const Entity& ref)->bool {return nullptr != ref.m_spDefence; });
		bool b距离友方单位太近 = false;
		if (!wp最近的友方单位.expired())
		{
			b距离友方单位太近 = m_refEntity.DistanceLessEqual(*wp最近的友方单位.lock(), 0.6f);
		}
		Entity& refTarget = *wpEntity.lock();

		if (m_refEntity.DistanceLessEqual(refTarget, 攻击距离(refTarget)) && !b距离友方单位太近)
		{
			走Component::Cancel所有包含走路的协程(m_refEntity); //TryCancel();

			if (m_类型 == 三色坦克)
			{
				if (co_await CoAttack位置(refTarget.Pos(), BuildingComponent::建筑半边长(refTarget), m_cancelAttack))
					co_return true;
			}
			else
			{
				if (co_await CoAttack目标(wpEntity, m_cancelAttack))
					co_return true;
			}

			continue;
		}
		else if (m_refEntity.m_wpOwner.expired() && m_refEntity.DistanceLessEqual(refTarget, m_refEntity.警戒距离()) &&
			//!走Component::正在走(m_refEntity) && 
			(!m_refEntity.m_sp采集 || m_refEntity.m_sp采集->m_TaskCancel.co.Finished()))
		{
			走Component::Cancel所有包含走路的协程(m_refEntity); //TryCancel();

			if (m_refEntity.m_sp采集)
			{
				m_refEntity.m_sp采集->m_TaskCancel.TryCancel();
			}

			if (co_await AiCo::WalkToTarget(m_refEntity, wpEntity.lock(), funCancel))
				co_return true;

			continue;
		}

		co_return false;
	}

	co_return false;
}

void AttackComponent::播放前摇动作()
{
	//switch (m_类型)
	//{
	//case 三色坦克:m_refEntity.BroadcastChangeSkeleAnim("attack_loop"); break;
	//default:m_refEntity.BroadcastChangeSkeleAnim("attack"); break;
	//}
	if (!m_战斗配置.str前摇动作.empty())
		m_refEntity.BroadcastChangeSkeleAnim(m_战斗配置.str前摇动作);
}

void AttackComponent::播放攻击动作()
{
	//switch (m_类型)
	//{
	//case 三色坦克:m_refEntity.BroadcastChangeSkeleAnim("attack_loop"); break;
	//default:m_refEntity.BroadcastChangeSkeleAnim("attack"); break;
	//}
	if (!m_战斗配置.str攻击动作.empty())
		m_refEntity.BroadcastChangeSkeleAnim(m_战斗配置.str攻击动作);
}

void AttackComponent::播放攻击音效()
{
	//switch (m_类型)
	//{
	//case 兵:
	//	if (!m_refEntity.m_spPlayer)
	//		EntitySystem::Broadcast播放声音(m_refEntity, "TMaFir00");
	//	break;
	//case 近战兵:EntitySystem::Broadcast播放声音(m_refEntity, "Tfrshoot"); break;
	//case 工程车:EntitySystem::Broadcast播放声音(m_refEntity, "TSCMin00"); break;
	//case 三色坦克:EntitySystem::Broadcast播放声音(m_refEntity, "音效/TTaFi200"); break;
	//}

	if (!m_战斗配置.str攻击音效.empty())
		EntitySystem::Broadcast播放声音(m_refEntity, m_战斗配置.str攻击音效);
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
	do
	{
		CHECK_终止攻击目标流程;

		m_refEntity.m_eulerAnglesY = CalculateAngle(m_refEntity.Pos(), wpDefencer.lock()->Pos());
		m_refEntity.BroadcastNotifyPos();
		播放前摇动作();

		if (0s < m_战斗配置.dura开始播放攻击动作 && co_await CoTimer::Wait(m_战斗配置.dura开始播放攻击动作, cancel))
			co_return true;//协程取消

		CHECK_终止攻击目标流程;

		播放攻击动作();
		if (0s < m_战斗配置.dura开始伤害 && co_await CoTimer::Wait(m_战斗配置.dura开始伤害, cancel))
			co_return true;//协程取消

		CHECK_终止攻击目标流程;

		auto& refDefencer = *wpDefencer.lock();
		if (!m_refEntity.DistanceLessEqual(refDefencer, 攻击距离(refDefencer)))
			break;//要执行后摇

		if (!refDefencer.m_spDefence)
			break;//目标打不了

		播放攻击音效();

		refDefencer.m_spDefence->受伤(m_战斗配置.f伤害);
	} while (false);

	if (co_await CoTimer::Wait(800ms, cancel))//后摇
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
		播放前摇动作();

		if (0s < m_战斗配置.dura开始播放攻击动作 && co_await CoTimer::Wait(m_战斗配置.dura开始播放攻击动作, cancel))
			co_return true;//协程取消

		CHECK_终止攻击位置流程;

		播放攻击动作();
		if (0s < m_战斗配置.dura开始伤害 && co_await CoTimer::Wait(m_战斗配置.dura开始伤害, cancel))
			co_return true;//协程取消

		CHECK_终止攻击位置流程;

		if (!m_refEntity.Pos().DistanceLessEqual(posTarget, 攻击距离(f目标建筑半边长)))
			break;//要执行后摇

		播放攻击音效();
		{
			SpEntity spEntity特效 = std::make_shared<Entity, const Position&, Space&, const 单位::单位配置&>(
				posTarget, m_refEntity.m_refSpace, { "特效" ,"特效/黄光爆闪","" });
			m_refEntity.m_refSpace.AddEntity(spEntity特效, 0);
			spEntity特效->BroadcastEnter();
			spEntity特效->CoDelayDelete().RunNew();
		}
		assert(m_refEntity.m_upAoi);
		if (m_refEntity.m_upAoi)
		{
			for (auto [k, wp] : m_refEntity.m_upAoi->m_map我能看到的)
			{
				CHECK_WP_CONTINUE(wp);
				auto& refDefencer = *wp.lock();
				if (refDefencer.m_spDefence && refDefencer.Pos().DistanceLessEqual(posTarget, 5))
					refDefencer.m_spDefence->受伤(m_战斗配置.f伤害);
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