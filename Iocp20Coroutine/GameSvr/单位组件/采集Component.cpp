#include "pch.h"
#include "采集Component.h"
#include "资源Component.h"
#include "BuildingComponent.h"
#include "Entity.h"
#include "../CoRoutine/CoTimer.h"
#include "../CoRoutine/CoEvent.h"
#include "Space.h"
#include "AiCo.h"
#include "MyEvent.h"
#include "AttackComponent.h"
#include "PlayerGateSession_Game.h"
#include "走Component.h"
#include "EntitySystem.h"
#include "PlayerComponent.h"
#include "造建筑Component.h"
#include "单位.h"
#include "PlayerNickNameComponent.h"

采集Component::采集Component(Entity& refEntity) : m_携带矿类型(晶体矿), m_refEntity(refEntity), m_目标资源类型(单位类型_Invalid_0)
{
}

void 采集Component::采集(WpEntity wp目标资源)
{
	if (造建筑Component::正在建造(m_refEntity))
	{
		PlayerComponent::播放声音Buzz(m_refEntity, "正在建造，不能采集");
		return;
	}
	走Component::Cancel所有包含走路的协程(m_refEntity);
	PlayerComponent::播放声音(m_refEntity, "语音/明白女声可爱版");
	m_TaskCancel.TryRun(Co采集(wp目标资源));
}

bool 资源可采集(WpEntity& refWp目标资源)
{
	if (refWp目标资源.expired())
		return false;

	auto& ref资源 = *refWp目标资源.lock();
	CHECK_FALSE(ref资源.m_up资源);
	if (ref资源.m_up资源->m_可采集数量 <= 0)
	{
		ref资源.DelayDelete();
		return false;
	}

	return true;
}
WpEntity 采集Component::Get目标资源(WpEntity& refWp目标资源)
{
	if (!资源可采集(refWp目标资源))
	{
		if (单位类型::单位类型_Invalid_0 == m_目标资源类型)
			return {};

		auto wp最近 = m_refEntity.Get最近的Entity(Entity::所有, m_目标资源类型);
		if (wp最近.expired())
			return {};

		if (!m_refEntity.DistanceLessEqual(*wp最近.lock(), 35))
			return {};

		refWp目标资源 = wp最近;
	}

	auto sp目标资源 = refWp目标资源.lock();
	CHECK_RET_DEFAULT(sp目标资源->m_up资源);
	m_目标资源类型 = sp目标资源->m_up资源->m_类型;
	return sp目标资源;
}

CoTaskBool 采集Component::Co采集(WpEntity wp目标资源)
{
	using namespace std;
	while (true)
	{
		//auto wpEntity基地 = m_refEntity.m_refSpace.Get最近的Entity支持地堡中的单位(m_refEntity, false,
		//	[](const Entity& ref)
		//	{
		//		return ref.m_spBuilding && ref.m_spBuilding->m_类型 == 基地;//找离自己最近的基地
		//	});
		if (m_refEntity.IsDead())
		{
			LOG(INFO) << "采集过程中自己阵亡";
			co_return false;
		}

		if (!m_refEntity.m_upPlayerNickName)
		{
			LOG(WARNING) << "采集过程中玩家断线";
			co_return false;
		}

		float min距离的平方 = std::numeric_limits<float>::max();
		WpEntity wpEntity基地;
		for (const auto [id, wp] : m_refEntity.m_refSpace.m_mapPlayer[m_refEntity.m_upPlayerNickName->m_strNickName].m_mapWpEntity)
		{
			CHECK_WP_CONTINUE(wp);
			const auto& refEntity = *wp.lock();
			if (!refEntity.m_upBuilding || (refEntity.m_类型 != 基地 && refEntity.m_类型 != 虫巢))//找离自己最近的基地
				continue;

			const auto pow2 = m_refEntity.DistancePow2(refEntity);
			if (min距离的平方 <= pow2)
				continue;

			min距离的平方 = pow2;
			wpEntity基地 = wp;
		}
		if (wpEntity基地.expired())
		{
			//if (co_await CoTimer::Wait(1s, m_TaskCancel.cancel))//自己连一个基地都没有
			//	co_return true;

			co_return false;
		}
		if (Max携带矿() <= m_u32携带矿)//装满了，回基地放矿
		{
			if (m_refEntity.DistanceLessEqual(*wpEntity基地.lock(), m_refEntity.攻击距离() + BuildingComponent::建筑半边长(*wpEntity基地.lock())))//在基地附近，满载矿，全部放进基地（直接加钱）
			{
				EntitySystem::BroadcastEntity描述(m_refEntity, "正在卸矿");
				if (co_await CoTimer::Wait(2s, m_TaskCancel.cancel))//把矿放进基地耗时
					co_return true;

				const auto u32携带矿 = m_u32携带矿;
				m_u32携带矿 = 0;
				auto wpEntity资源 = Get目标资源(wp目标资源);
				if (wpEntity资源.expired())
				{
					提醒资源枯竭();
					co_return false;//目标资源没了
				}

				Entity& ref资源 = *wpEntity资源.lock();
				if (ref资源.m_类型 == 晶体矿)
				{
					//const auto& [stop, _] = co_await AiCo::ChangeMoney(refGateSession, addMoney, true, m_TaskCancel.cancel);
					//if (stop)
					//	co_return true;
					Space::GetSpacePlayer(m_refEntity).m_u32晶体矿 += u32携带矿;
					CoEvent<MyEvent::晶体矿已运回基地>::OnRecvEvent({});
				}
				else
				{
					Space::GetSpacePlayer(m_refEntity).m_u32燃气矿 += u32携带矿;
				}
				PlayerComponent::Send资源(m_refEntity);
				continue;
			}

			//离基地太远，走向基地
			//m_refEntity.m_upAttack->TryCancel();
			EntitySystem::停止攻击和治疗(m_refEntity);

			CHECK_CO_RET_FALSE(m_refEntity.m_up走);
			if (co_await m_refEntity.m_up走->WalkToTarget(wpEntity基地, m_TaskCancel.cancel, false))
				co_return true;//中断，可能打怪去了

			continue;
		}

		//还没装满，还要继续去采矿
		{
			{
				auto wpEntity资源 = Get目标资源(wp目标资源);
				if (wpEntity资源.expired())
				{
					提醒资源枯竭();
					co_return false;//目标资源没了
				}
				Entity& ref资源 = *wpEntity资源.lock();
				if (!m_refEntity.DistanceLessEqual(ref资源, m_refEntity.攻击距离()))
				{
					CHECK_CO_RET_FALSE(ref资源.m_up资源);
					EntitySystem::BroadcastEntity描述(m_refEntity, std::format("走向{0}", ref资源.m_up资源->m_类型 == 晶体矿 ? "晶体矿" : "燃气矿"));
					//m_refEntity.m_upAttack->TryCancel();

					CHECK_CO_RET_FALSE(m_refEntity.m_up走);
					if (co_await m_refEntity.m_up走->WalkToTarget(wp目标资源, m_TaskCancel.cancel, false))
						co_return true;//中断

					m_refEntity.BroadcastChangeSkeleAnim(m_refEntity.m_类型 == 工虫 ? "采集" : "2", true);
				}
			}

			//在目标矿附近
			CoEvent<MyEvent::开始采集晶体矿>::OnRecvEvent({});
			if (co_await CoTimer::Wait(1s, m_TaskCancel.cancel))//采矿1个矿耗时
				co_return true;//中断

			auto wpEntity资源 = Get目标资源(wp目标资源);
			if (wpEntity资源.expired())
			{
				提醒资源枯竭();
				co_return false;//目标资源已采空
			}
			Entity& ref资源 = *wpEntity资源.lock();
			
			if (ref资源.m_类型 != m_携带矿类型)
			{
				m_u32携带矿 = 0;
				m_携带矿类型 = ref资源.m_类型;
			}
			if (0 == m_u32携带矿)
			{
				switch (ref资源.m_类型)
				{
				case 晶体矿:PlayerComponent::播放声音(m_refEntity, "TSCMin00"); break;
				case 燃气矿:PlayerComponent::播放声音(m_refEntity, "TSCMin01"); break;
				default:break;
				}
			}

			CHECK_CO_RET_FALSE(ref资源.m_up资源);
			--ref资源.m_up资源->m_可采集数量;
			EntitySystem::BroadcastEntity描述(ref资源, std::format("剩余:{0}", ref资源.m_up资源->m_可采集数量));

			++m_u32携带矿;
			EntitySystem::BroadcastEntity描述(m_refEntity, std::format("已采集{0}", m_u32携带矿));
			continue;
		}
	}
}

void 采集Component::提醒资源枯竭()
{
	if (晶体矿 == m_携带矿类型)
		PlayerComponent::播放声音(m_refEntity, m_refEntity.m_类型 == 工程车 ? "语音/晶体矿已枯竭女声可爱版" : "语音/晶体矿已枯竭_男声", "晶体矿 已枯竭");
	else
		PlayerComponent::播放声音(m_refEntity, m_refEntity.m_类型 == 工程车 ? "语音/燃气矿已枯竭女声可爱版" : "语音/燃气矿已枯竭_男声", "燃气矿 已枯竭");

}
void 采集Component::AddComponent(Entity& refEntity)
{
	refEntity.AddComponentOnDestroy(&Entity::m_up采集, refEntity);
}

bool 采集Component::正在采集(Entity& refEntity)
{
	if (!refEntity.m_up采集)
		return false;

	return !refEntity.m_up采集->m_TaskCancel.co.Finished();
}

void 采集Component::OnEntityDestroy(const bool bDestroy)
{
	m_TaskCancel.TryCancel();
}