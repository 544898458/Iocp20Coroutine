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

extern std::unordered_map<int, uint64_t> m_mapEntityId;
AttackComponent& AttackComponent::AddComponent(Entity& refEntity)
{
	if (refEntity.m_spAttack)
	{
		LOG(ERROR) << "不能重复加AttackComponent";
		_ASSERT(!"不能重复加AttackComponent");
		return *refEntity.m_spAttack;
	}
	refEntity.m_spAttack = std::make_shared<AttackComponent, Entity&>(refEntity);
	//refEntity.m_spAttack->m_f攻击距离 = f攻击距离;
	//refEntity.m_spAttack->m_f警戒距离 = f警戒距离;
	//refEntity.m_spAttack->m_f伤害 = f伤害;
	//float arrF[] = { refEntity.Pos().x,0,refEntity.Pos().z};
	//int CrowToolAddAgent(float arrF[]);
	//refEntity.m_spAttack->m_idxCrowdAgent = CrowToolAddAgent(arrF);
	//m_mapEntityId[refEntity.m_spAttack->m_idxCrowdAgent] = refEntity.Id;
	refEntity.m_spAttack->Co顶层().RunNew();
	return *refEntity.m_spAttack;
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
Position AttackComponent::怪物闲逛(const Position& refOld)
{
	auto posTarget = refOld;
	posTarget.x += std::rand() % 11 - 5;//随机走
	posTarget.z += std::rand() % 11 - 5;
	return posTarget;
}
using namespace std;
AttackComponent::AttackComponent(Entity& refEntity) :
	m_refEntity(refEntity)
{
	CHECK_RET_VOID(单位::Find战斗配置(refEntity.m_类型, m_战斗配置));
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

		if (!可以攻击())
			continue;

		if ((m_b搜索新的目标) && co_await Co走向警戒范围内的目标然后攻击())
			continue;

		if (!m_b原地坚守 && m_refEntity.m_sp走 && m_fun空闲走向此处 && !走Component::正在走(m_refEntity))//打完走向下一个目标
		{
			走Component::Cancel所有包含走路的协程(m_refEntity); //TryCancel();

			auto posTarget = m_fun空闲走向此处(m_refEntity.Pos());
			m_refEntity.m_refSpace.CrowdToolFindNerestPos(posTarget);
			if (m_refEntity.Pos().DistanceLessEqual(posTarget, 3))
			{
				m_fun空闲走向此处 = m_refEntity.m_spPlayerNickName ? nullptr : 怪物闲逛;
				continue;
			}

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
	if (m_refEntity.m_spBuilding && !m_refEntity.m_spBuilding->已造好())
		return false;

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

bool AttackComponent::检查穿墙(const Position& pos)
{
	if (绿色坦克 != m_refEntity.m_类型)
		return true;

	return m_refEntity.m_refSpace.CrowdTool可走直线(m_refEntity.Pos(), pos);
}

CoTaskBool AttackComponent::Co走向警戒范围内的目标然后攻击()
{
	KeepCancel kc(m_TaskCancel.cancel);
	while (true)
	{
		if (co_await CoTimer::WaitNextUpdate(m_TaskCancel.cancel))
			co_return true;

		if (!可以攻击())
			co_return false;

		auto wpEntity = m_refEntity.Get最近的Entity支持地堡中的单位(Entity::敌方, [this](const Entity& ref)->bool
			{
				if (!EntitySystem::Is空地能打(m_refEntity.m_类型, ref.m_类型))
					return false;
				if (!ref.m_spDefence)
					return false;
				return nullptr != ref.m_spDefence;
			});
		if (wpEntity.expired())
		{
			m_b搜索新的目标 = false;//警戒范围内没有目标
			co_return false;
		}

		const bool b距离友方单位太近 = EntitySystem::距离友方单位太近(m_refEntity);
		Entity& refTarget = *wpEntity.lock();

		if (m_refEntity.DistanceLessEqual(refTarget, 攻击距离(refTarget)) && !b距离友方单位太近 && 检查穿墙(refTarget.Pos()))
		{
			走Component::Cancel所有包含走路的协程(m_refEntity); //TryCancel();

			if (m_refEntity.m_类型 == 三色坦克)
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

		if (m_b原地坚守)
			continue;

		bool b仇恨目标 = false;

		//仇恨列表
		if (m_refEntity.m_spDefence)
		{
			auto& refMap = m_refEntity.m_spDefence->m_map对我伤害;
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

				//wpEntity = wp;
				b仇恨目标 = true;
				break;
			}
		}

		if (m_refEntity.m_wpOwner.expired() && (b仇恨目标 || m_refEntity.DistanceLessEqual(refTarget, m_refEntity.警戒距离())) &&
			//!走Component::正在走(m_refEntity) && 
			(!m_refEntity.m_sp采集 || m_refEntity.m_sp采集->m_TaskCancel.co.Finished()))
		{
			走Component::Cancel所有包含走路的协程(m_refEntity); //TryCancel();

			if (m_refEntity.m_sp采集)
			{
				m_refEntity.m_sp采集->m_TaskCancel.TryCancel();
			}

			if (co_await AiCo::WalkToTarget(m_refEntity, wpEntity.lock(), m_TaskCancel.cancel, !b仇恨目标))
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
	EntitySystem::恢复休闲动作 _(m_refEntity, {});

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

		switch (m_refEntity.m_类型)
		{
		case 炮台:
			m_refEntity.Broadcast<Msg弹丸特效>({ .idEntity = m_refEntity.Id, .idEntityTarget = wpDefencer.lock()->Id,.特效name = StrConv::GbkToUtf8("特效/黄泡泡") });
			break;
		default:; break;
		}

		CHECK_终止攻击目标流程;

		auto& refDefencer = *wpDefencer.lock();
		if (!m_refEntity.DistanceLessEqual(refDefencer, 攻击距离(refDefencer)))
			break;//要执行后摇

		if (!refDefencer.m_spDefence)
			break;//目标打不了

		播放攻击音效();

		if (0 < m_战斗配置.i32伤害)
			refDefencer.m_spDefence->受伤(m_战斗配置.i32伤害, m_refEntity.Id);

		if (绿色坦克 == m_refEntity.m_类型)
		{
			auto wp光刺 = m_refEntity.m_refSpace.造活动单位(m_refEntity.m_spPlayer, EntitySystem::GetNickName(m_refEntity), m_refEntity.Pos(), 光刺);
			CHECK_WP_CO_RET_FALSE(wp光刺);
			飞向目标Component::AddComponet(*wp光刺.lock(), m_refEntity.Pos(),  (refDefencer.Pos()-m_refEntity.Pos()).归一化(), m_战斗配置.f攻击距离);
		}
	} while (false);

	if (co_await CoTimer::Wait(m_战斗配置.dura后摇, cancel))//后摇
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
		EntitySystem::BroadcastEntity描述(m_refEntity, "准备开炮");
		{
			SpEntity spEntity特效 = std::make_shared<Entity, const Position&, Space&, 单位类型, const 单位::单位配置&>(
				posTarget, m_refEntity.m_refSpace, 特效, { "炸点" ,"特效/炸点","" });
			m_refEntity.m_refSpace.AddEntity(spEntity特效, 0);
			spEntity特效->BroadcastEnter();
			spEntity特效->CoDelayDelete(m_战斗配置.dura开始播放攻击动作 + m_战斗配置.dura开始伤害).RunNew();
		}
		if (0s < m_战斗配置.dura开始播放攻击动作 && co_await CoTimer::Wait(m_战斗配置.dura开始播放攻击动作, cancel))
			co_return true;//协程取消

		CHECK_终止攻击位置流程;
		EntitySystem::BroadcastEntity描述(m_refEntity, "");
		播放攻击动作();
		if (0s < m_战斗配置.dura开始伤害 && co_await CoTimer::Wait(m_战斗配置.dura开始伤害, cancel))
			co_return true;//协程取消

		CHECK_终止攻击位置流程;

		if (!m_refEntity.Pos().DistanceLessEqual(posTarget, 攻击距离(f目标建筑半边长)))
			break;//要执行后摇

		播放攻击音效();
		{
			SpEntity spEntity特效 = std::make_shared<Entity, const Position&, Space&, 单位类型, const 单位::单位配置&>(
				posTarget, m_refEntity.m_refSpace, 特效, { "爆炸溅射" ,"特效/黄光爆闪","" });
			m_refEntity.m_refSpace.AddEntity(spEntity特效, 0);
			spEntity特效->BroadcastEnter();
			spEntity特效->CoDelayDelete(1s).RunNew();
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

				if (refDefencer.m_spDefence && refDefencer.Pos().DistanceLessEqual(posTarget, 5))
					refDefencer.m_spDefence->受伤(m_战斗配置.i32伤害, m_refEntity.Id);
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