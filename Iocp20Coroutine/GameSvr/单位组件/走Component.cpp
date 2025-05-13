#include "pch.h"
#include "走Component.h"
#include "../Entity.h"
#include "../EntitySystem.h"
#include "../../CoRoutine/CoTimer.h"
#include "../../CoRoutine/CoEvent.h"
#include "PlayerComponent.h"
#include "AttackComponent.h"
#include "AiCo.h"
#include "MonsterComponent.h"
#include "Space.h"
#include "可进活动单位Component.h"
#include "采集Component.h"
#include "造建筑Component.h"
#include "BuildingComponent.h"
#include "找目标走过去Component.h"
#include "医疗兵Component.h"
#include "临时阻挡Component.h"
#include "../RecastNavigationCrowd.h"

void 走Component::AddComponent(Entity& refEntity)
{
	CHECK_RET_VOID(!refEntity.m_up走);
	refEntity.AddComponentOnDestroy(&Entity::m_up走, refEntity);
	//float arrF[] = { refEntity.Pos().x,0,refEntity.Pos().z};
	//int CrowToolAddAgent(float arrF[]);
	//refEntity.m_upAttack->m_idxCrowdAgent = CrowToolAddAgent(arrF);
	//m_mapEntityId[refEntity.m_upAttack->m_idxCrowdAgent] = refEntity.Id;
}

走Component::走Component(Entity& refEntity) :m_refEntity(refEntity)
{
}

bool 走Component::正在走(const Entity& refEntity)
{
	if (!refEntity.m_up走)
		return false;

	return !refEntity.m_up走->m_coWalk.Finished();
}

void 走Component::WalkToTarget(SpEntity spTarget)
{
	_ASSERT(!m_cancel);
	_ASSERT(m_coWalk.Finished());
	m_coWalk = WalkToTarget(spTarget, m_cancel);
	m_coWalk.Run();
}

/// <summary>
/// 
/// </summary>
/// <param name="refThis"></param>
/// <param name="localTarget"></param>
/// <returns>是否还要走下一步</returns>
bool 已走到目标附近(Entity& refThis, const Position localTarget, const float f距离目标小于此距离停下 = 0)
{
	CHECK_RET_DEFAULT(refThis.m_up走);
	const float step = std::max(EntitySystem::升级后速度每帧移动距离(refThis), f距离目标小于此距离停下);
	if (refThis.Pos().DistanceLessEqual(localTarget, step))
	{
		//LOG(INFO) << "已走到" << localTarget.x << "," << localTarget.z << "附近，协程正常退出";
		//EntitySystem::BroadcastChangeSkeleAnimIdle(refThis);
		return true;
	}

	//refThis.m_eulerAnglesY = CalculateAngle(refThis.Pos(), localTarget);
	refThis.BroadcastNotifyPos();

	return false;
}

CoTaskBool 走Component::WalkToTarget(SpEntity spTarget, FunCancel& funCancel, const bool b检查警戒距离, const std::function<bool(Entity&)>& fun可停下)
{
	//if (!refThis.m_up走)
	//	co_return false;

	if (m_refEntity.IsDead())
	{
		//LOG(WARNING) << m_refEntity.头顶Name() << ",已阵亡不用走";
		co_return false;
	}

	auto posTarget = spTarget->Pos();
	{
		const auto ok = m_refEntity.m_refSpace.CrowdToolFindNerestPos(posTarget);
		LOG_IF(ERROR, !ok) << "";
		_ASSERT(ok);
	}
	活动单位走完路加阻挡 _(m_refEntity);
	RecastNavigationCrowd rnc(m_refEntity, posTarget);
	KeepCancel kc(funCancel);
	const float f建筑半边长 = BuildingComponent::建筑半边长(*spTarget);


	//refThis.BroadcastChangeSkeleAnim("run");
	单位::活动单位配置 配置;
	单位::Find活动单位配置(m_refEntity.m_类型, 配置);
	EntitySystem::恢复休闲动作 __(m_refEntity, 配置.str走路动作);
	Position posOld;
	CoEvent<MyEvent::MoveEntity>::OnRecvEvent({ m_refEntity.weak_from_this() });
	while (true)
	{
		if (co_await CoTimer::WaitNextUpdate(funCancel))//服务器主工作线程大循环，每次循环触发一次
		{
			//LOG(INFO) << "走向" << spTarget << "的协程取消了";
			co_return true;
		}
		if (m_refEntity.IsDead())
		{
			//LOG(INFO) << "自己阵亡，走向[" << spTarget->头顶Name() << "]的协程取消了";
			//PlayerComponent::Say(m_refEntity, "自己阵亡", SayChannel::系统);

			co_return false;
		}
		if (b检查警戒距离 && !m_refEntity.DistanceLessEqual(*spTarget, m_refEntity.警戒距离()))
		{
			//LOG(INFO) << "离开自己的警戒距离" << spTarget << "的协程取消了";
			co_return false;
		}
		const bool b距离友方单位太近 = EntitySystem::距离友方单位太近(m_refEntity);
		if (!b距离友方单位太近 && m_refEntity.DistanceLessEqual(*spTarget, m_refEntity.攻击距离() + f建筑半边长) && fun可停下(*spTarget))
		{
			//LOG(INFO) << "已走到" << spTarget << "附近，协程正常退出";
			//EntitySystem::BroadcastChangeSkeleAnimIdle(m_refEntity);
			//EntitySystem::BroadcastEntity描述(m_refEntity, "已走到目标附近");
			co_return false;
		}

		if (posOld != spTarget->Pos())
		{
			rnc.SetMoveTarget(spTarget->Pos());
			posOld = spTarget->Pos();
		}

		if (已走到目标附近(m_refEntity, spTarget->Pos()))
		{
			co_return false;
		}
		//EntitySystem::BroadcastEntity描述(m_refEntity, std::format("距目标{0}米", (int)m_refEntity.Distance(*spTarget)));
	}
	LOG(INFO) << "走向目标协程结束:" << m_refEntity.Pos();
	co_return false;
}

bool 走Component::WalkToTarget(Entity& refThis, SpEntity spTarget)
{
	CHECK_RET_FALSE(refThis.m_up走);
	refThis.m_up走->WalkToTarget(spTarget);
	return true;
}

void 走Component::WalkToPos(const Position& posTarget)
{
	_ASSERT(!m_cancel);
	_ASSERT(m_coWalk.Finished());
	m_coWalk = WalkToPos(posTarget, m_cancel);
	m_coWalk.Run();//协程离开开始运行（运行到第一个co_await
}

bool 走Component::WalkToPos(Entity& refThis, const Position& posTarget)
{
	CHECK_RET_FALSE(refThis.m_up走);
	refThis.m_up走->WalkToPos(posTarget);
	return true;
}

CoTaskBool 走Component::WalkToPos(const Position posTarget, FunCancel& funCancel, const float f距离目标小于此距离停下)
{
	if (m_refEntity.IsDead())
	{
		//LOG(WARNING) << posTarget << ",已阵亡不用走";
		co_return false;
	}
	if (!m_refEntity.m_refSpace.CrowdTool可站立(posTarget))
	{
		LOG(INFO) << posTarget << "不可站立";
		co_return false;
	}
	const auto posOld = m_refEntity.Pos();
	活动单位走完路加阻挡 _(m_refEntity);
	std::shared_ptr<RecastNavigationCrowd> sp;
	if (m_wpRecastNavigationCrowd.expired())
	{
		sp = std::make_shared<RecastNavigationCrowd, Entity&, const Position&>(m_refEntity, posTarget);
	}
	else
	{
		sp = m_wpRecastNavigationCrowd.lock();
		sp->SetMoveTarget(posTarget);
		//ref.SetSpeed();
	}


	m_wpRecastNavigationCrowd = sp;
	KeepCancel kc(funCancel);
	const auto posLocalTarget = posTarget;
	//m_refEntity.BroadcastChangeSkeleAnim("run");
	单位::活动单位配置 配置;
	单位::Find活动单位配置(m_refEntity.m_类型, 配置);
	EntitySystem::恢复休闲动作 __(m_refEntity, 配置.str走路动作);
	CoEvent<MyEvent::MoveEntity>::OnRecvEvent({ m_refEntity.weak_from_this() });
	while (true)
	{
		if (co_await CoTimer::WaitNextUpdate(funCancel))//服务器主工作线程大循环，每次循环触发一次
		{
			//LOG(INFO) << "走向" << posLocalTarget << "的协程取消了";
			co_return true;
		}
		if (m_refEntity.IsDead())
		{
			//LOG(INFO) << "自己阵亡，走向" << posLocalTarget << "的协程取消了";
			//PlayerComponent::Say(m_refEntity, "自己阵亡", SayChannel::系统);

			co_return true;
		}

		if (已走到目标附近(m_refEntity, posLocalTarget, f距离目标小于此距离停下))
		{
			if (m_refEntity.m_upPlayer)
			{
				int a = 0;
			}

			for (int i = 0; i < 30; ++i)
			{
				auto wp最近 = m_refEntity.Get最近的Entity(Entity::友方, [](const Entity&) {return true; });
				if (wp最近.expired())
					co_return false;

				Entity& ref最近 = *wp最近.lock();

				if (!m_refEntity.DistanceLessEqual(ref最近, 2.0f))
				{
					//LOG(INFO) << "附近没有友方单位，停止走向" << posLocalTarget;
					co_return false;
				}
				if (co_await CoTimer::WaitNextUpdate(funCancel))//服务器主工作线程大循环，每次循环触发一次
				{
					//LOG(INFO) << "已走到目标附近，走向" << posLocalTarget << "的协程取消了";
					co_return true;
				}
			}

			LOG(INFO) << "已走到目标附近，附近都是友方单位," << posLocalTarget;
			co_return false;
		}

		//EntitySystem::BroadcastEntity描述(refThis, std::format("距目标{0}米", (int)refThis.Pos().Distance(posTarget)));
	}
	LOG(INFO) << "走向目标协程结束:" << posTarget;
	co_return false;
}

void 走Component::WalkToPos手动控制(const Position& posTarget)
{
	//void CrowdToolSetMoveTarget(const float* p, const int idx);
	//float arrFloat[] = { posTarget.x,0,posTarget.z };
	//CrowdToolSetMoveTarget(arrFloat, refEntity.m_upAttack->m_idxCrowdAgent);
	//return;
	if (m_refEntity.IsDead())
	{
		//PlayerComponent::Say(m_refEntity, "自己阵亡,不能走", SayChannel::系统);

		return;
	}
	//m_coStop = true;

	OnEntityDestroy(false);

	m_coWalk.Run();
	_ASSERT(m_coWalk.Finished());//20240205
	//if (m_refEntity.m_upAttack)
	//{
	//	m_refEntity.m_upAttack->OnEntityDestroy(const bool bDestroy);
	//	//_ASSERT(m_refEntity.m_upAttack->m_coAttack.Finished());//20240205
	//}
	EntitySystem::停止攻击和治疗(m_refEntity);
	/*m_coStop = false;*/
	_ASSERT(!m_cancel);
	m_coWalk手动控制 = WalkToPos(posTarget, m_cancel);
	m_coWalk手动控制.Run();//协程离开开始运行（运行到第一个co_await
}


void 走Component::OnEntityDestroy(const bool bDestroy)
{
	if (m_cancel)
	{
		//LOG(INFO) << "调用m_cancel";
		m_cancel.TryCancel();
	}
	else
	{
		//LOG(INFO) << "m_cancel是空的，没有要取消的协程";
		if (!m_coWalk.Finished() || !m_coWalk手动控制.Finished())
		{
			LOG(ERROR) << "协程没结束，却提前清空了m_cancel";
			_ASSERT(false);
		}
	}

	_ASSERT(m_coWalk.Finished());//20240205
	//_ASSERT(m_coAttack.Finished());//20240205

}

void 走Component::走进(WpEntity wpEntity地堡)
{
	//if (m_refEntity.m_upAttack)
	//	m_refEntity.m_upAttack->OnEntityDestroy(const bool bDestroy);

	//if (m_refEntity.m_up找目标走过去)
	//	m_refEntity.m_up找目标走过去->OnEntityDestroy(const bool bDestroy);

	//if (m_refEntity.m_up医疗兵)
	//	m_refEntity.m_up医疗兵->OnEntityDestroy(const bool bDestroy);
	EntitySystem::停止攻击和治疗(m_refEntity);

	if (wpEntity地堡.expired())
		return;

	if (wpEntity地堡.lock()->m_upBuilding && !wpEntity地堡.lock()->m_upBuilding->已造好())
	{
		PlayerComponent::播放声音Buzz(m_refEntity, "地堡还没造好，不能进地堡");
		return;
	}

	//if (造建筑Component::正在建造(m_refEntity))
	//{
	//	PlayerComponent::播放声音Buzz(m_refEntity, "正在建造，不能进地堡");
	//	return;
	//}

	//if (m_refEntity.m_upAttack)
	//	m_refEntity.m_upAttack->OnEntityDestroy(const bool bDestroy);
	EntitySystem::停止攻击和治疗(m_refEntity);
	Cancel所有包含走路的协程(m_refEntity);
	_ASSERT(!m_cancel);
	_ASSERT(m_coWalk进地堡.Finished());
	m_coWalk进地堡 = Co走进地堡(wpEntity地堡);
	m_coWalk进地堡.Run();
}

CoTaskBool 走Component::Co走进地堡(WpEntity wpEntity地堡)
{
	KeepCancel kc(m_cancel);

	if (m_refEntity.m_upAttack)
	{
		switch (m_refEntity.m_类型)
		{
		case 工程车:PlayerComponent::播放声音(m_refEntity, "语音/明白女声可爱版", ""); break;
		case 枪兵:PlayerComponent::播放声音(m_refEntity, "语音/明白男声正经版", ""); break;
		default:break;
		}
	}
	while (!wpEntity地堡.expired())
	{
		if (m_refEntity.IsDead())
			co_return false;

		auto spEntity地堡 = wpEntity地堡.lock();
		if (!spEntity地堡->m_up可进活动单位)
			co_return false;

		if (m_refEntity.DistanceLessEqual(*spEntity地堡, m_refEntity.攻击距离() + BuildingComponent::建筑半边长(*spEntity地堡)))
		{
			spEntity地堡->m_up可进活动单位->进(m_refEntity.m_refSpace, m_refEntity);

			co_return false;
		}

		_ASSERT(!m_cancel);
		if (co_await WalkToTarget(spEntity地堡, m_cancel, false))
			co_return true;

	}

	co_return false;
}

void 走Component::Cancel所有包含走路的协程(Entity& refEntity, const bool b停止攻击)
{
	if (b停止攻击)EntitySystem::停止攻击和治疗(refEntity);
	if (refEntity.m_up采集)		refEntity.m_up采集->m_TaskCancel.TryCancel();
	if (refEntity.m_up走)		refEntity.m_up走->OnEntityDestroy(false);
	if (refEntity.m_up造建筑)	refEntity.m_up造建筑->OnEntityDestroy(false);
}