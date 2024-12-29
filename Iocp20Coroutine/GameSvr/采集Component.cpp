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

采集Component::采集Component(Entity& refEntity) : m_携带矿类型(晶体矿), m_refEntity(refEntity)
{
}

void 采集Component::采集(PlayerGateSession_Game& refGateSession, WpEntity wp目标资源)
{
	if (造建筑Component::正在建造(m_refEntity))
	{
		PlayerComponent::播放声音(m_refEntity, "BUZZ", "正在建造，不能采集");
		return;
	}
	走Component::Cancel所有包含走路的协程(m_refEntity);
	PlayerComponent::播放声音(m_refEntity, "语音/明白女声可爱版");
	m_TaskCancel.TryRun(Co采集(refGateSession, wp目标资源));
}

std::tuple<std::shared_ptr<Entity>, std::shared_ptr<资源Component>> Get目标资源(WpEntity& refWp目标资源)
{
	if (refWp目标资源.expired())
		return { {},{} };

	auto sp目标资源 = refWp目标资源.lock();
	auto sp资源 = sp目标资源->m_sp资源;
	return { sp目标资源 ,sp资源 };
}

CoTaskBool 采集Component::Co采集(PlayerGateSession_Game& refGateSession, WpEntity wp目标资源)
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

		if (!m_refEntity.m_spPlayerNickName)
		{
			LOG(WARNING) << "采集过程中玩家断线";
			co_return false;
		}

		float min距离的平方 = std::numeric_limits<float>::max();
		WpEntity wpEntity基地;
		for (const auto [id, wp] : m_refEntity.m_refSpace.m_mapPlayer[m_refEntity.m_spPlayerNickName->m_strNickName].m_mapWpEntity)
		{
			CHECK_WP_CONTINUE(wp);
			const auto& refEntity = *wp.lock();
			if (!refEntity.m_spBuilding || refEntity.m_spBuilding->m_类型 != 基地)//找离自己最近的基地
				continue;

			const auto pow2 = m_refEntity.DistancePow2(refEntity);
			if (min距离的平方 <= pow2)
				continue;

			min距离的平方 = pow2;
			wpEntity基地 = wp;
		}
		if (wpEntity基地.expired())
		{
			if (co_await CoTimer::Wait(1s, m_TaskCancel.cancel))//自己连一个基地都没有，等一会儿再试
				co_return true;
			continue;
		}
		if (Max携带矿() <= m_u32携带矿)//装满了，回基地放矿
		{
			if (m_refEntity.DistanceLessEqual(*wpEntity基地.lock(), m_refEntity.攻击距离() + BuildingComponent::建筑半边长(*wpEntity基地.lock())))//在基地附近，满载矿，全部放进基地（直接加钱）
			{
				if (co_await CoTimer::Wait(1s, m_TaskCancel.cancel))//把矿放进基地耗时
					co_return true;

				auto addMoney = m_u32携带矿;
				m_u32携带矿 = 0;
				auto [_, sp资源] = Get目标资源(wp目标资源);
				if (!sp资源)
					co_return false;//目标资源没了

				if (sp资源->m_类型 == 晶体矿)
				{
					const auto& [stop, _] = co_await AiCo::ChangeMoney(refGateSession, addMoney, true, m_TaskCancel.cancel);
					if (stop)
						co_return true;

					CoEvent<MyEvent::晶体矿已运回基地>::OnRecvEvent(false, {});
				}
				else
				{
					Space::GetSpacePlayer(m_refEntity).m_u32燃气矿 += addMoney;
					refGateSession.Send资源();
				}
				continue;
			}

			//离基地太远，走向基地
			m_refEntity.m_spAttack->TryCancel();
			if (co_await AiCo::WalkToTarget(m_refEntity, wpEntity基地.lock(), m_TaskCancel.cancel, false))
				co_return true;//中断，可能打怪去了

			continue;
		}

		//还没装满，还要继续去采矿
		{
			{
				auto [spEntity资源, _] = Get目标资源(wp目标资源);
				if (!spEntity资源)
					co_return false;//目标资源没了

				if (!m_refEntity.DistanceLessEqual(*spEntity资源, m_refEntity.攻击距离()))
				{
					//距离目标矿太远，走向晶体矿
					//m_refEntity.m_spAttack->TryCancel();
					if (co_await AiCo::WalkToTarget(m_refEntity, wp目标资源.lock(), m_TaskCancel.cancel, false))
						co_return true;//中断
				}
			}

			//在目标矿附近

			CoEvent<MyEvent::开始采集晶体矿>::OnRecvEvent(false, {});
			if (co_await CoTimer::Wait(1s, m_TaskCancel.cancel))//采矿1个矿耗时
				co_return true;//中断

			auto [spEntity资源, sp资源] = Get目标资源(wp目标资源);
			if (!spEntity资源 || !sp资源)
				co_return false;//目标资源没了

			if (sp资源->m_可采集数量 <= 0)
			{
				spEntity资源->CoDelayDelete().RunNew();
				co_return false;//目标资源已采空
			}

			if (sp资源->m_类型 != m_携带矿类型)
			{
				m_u32携带矿 = 0;
				m_携带矿类型 = sp资源->m_类型;
			}
			if (0 == m_u32携带矿)
			{
				switch (sp资源->m_类型)
				{
				case 晶体矿:PlayerComponent::播放声音(m_refEntity, "TSCMin00"); break;
				case 燃气矿:PlayerComponent::播放声音(m_refEntity, "TSCMin01"); break;
				default:break;
				}
			}

			--sp资源->m_可采集数量;
			EntitySystem::BroadcastEntity描述(*spEntity资源, std::format("剩余:{0}", sp资源->m_可采集数量));

			++m_u32携带矿;
			EntitySystem::BroadcastEntity描述(m_refEntity, std::format("已采集{0}", m_u32携带矿));
			continue;



		}
	}
}

void 采集Component::AddComponent(Entity& refEntity)
{
	refEntity.m_sp采集 = std::make_shared<采集Component, Entity&>(refEntity);
}

