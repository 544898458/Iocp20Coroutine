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

采集Component::采集Component(Entity &refEntity) : m_携带矿类型(晶体矿), m_refEntity(refEntity)
{
}

void 采集Component::采集(PlayerGateSession_Game& refGateSession, WpEntity wp目标资源)
{
	走Component::Cancel所有包含走路的协程(m_refEntity);
	m_TaskCancel.TryRun( Co采集(refGateSession, wp目标资源));
}

CoTaskBool 采集Component::Co采集(PlayerGateSession_Game& refGateSession, WpEntity wp目标资源)
{
	using namespace std;
	while (true)
	{
		if (wp目标资源.expired())
			co_return false;//目标资源没了

		auto sp目标资源 = wp目标资源.lock();
		auto sp资源 = sp目标资源->m_sp资源;
		CHECK_CO_RET_FALSE(sp资源);
		

		auto wpEntity基地 = m_refEntity.m_refSpace.Get最近的Entity(m_refEntity, false,
			[](const Entity& ref)
			{
				return ref.m_spBuilding && ref.m_spBuilding->m_类型 == 基地;//找离自己最近的基地
			});

		if (wpEntity基地.expired())
		{
			if (co_await CoTimer::Wait(1s, m_TaskCancel.cancel))//自己连一个基地都没有，等一会儿再试
				co_return true;
			continue;
		}

		if (Max携带矿() <= m_u32携带矿)//装满了，回基地放矿
		{
			if (m_refEntity.DistanceLessEqual(*wpEntity基地.lock(), m_refEntity.攻击距离()))//在基地附近，满载矿，全部放进基地（直接加钱）
			{
				if (co_await CoTimer::Wait(1s, m_TaskCancel.cancel))//把矿放进基地耗时
					co_return true;

				auto addMoney = m_u32携带矿;
				m_u32携带矿 = 0;
				if (sp资源->m_类型 == 晶体矿)
				{
					const auto& [stop, _] = co_await AiCo::ChangeMoney(refGateSession, addMoney, true, m_TaskCancel.cancel);
					if (stop)
						co_return true;

					CoEvent<MyEvent::晶体矿已运回基地>::OnRecvEvent(false, {});
				}
				else
				{
					refGateSession.m_u32燃气矿 += addMoney;
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
		
		if (m_refEntity.DistanceLessEqual(*sp目标资源, m_refEntity.攻击距离()))//在目标矿附近
		{
			CoEvent<MyEvent::开始采集晶体矿>::OnRecvEvent(false, {});
			if(co_await CoTimer::Wait(1s, m_TaskCancel.cancel))//采矿1个矿耗时
				co_return true;//中断

			if (sp资源->m_类型 != m_携带矿类型)
				m_u32携带矿;

			++m_u32携带矿;
			continue;
		}

		//距离目标矿太远，走向晶体矿
		//m_refEntity.m_spAttack->TryCancel();
		if (co_await AiCo::WalkToTarget(m_refEntity, wp目标资源.lock(), m_TaskCancel.cancel, false))
			co_return true;//中断
	}
}

void 采集Component::AddComponent(Entity& refEntity)
{
	refEntity.m_sp采集 = std::make_shared<采集Component,Entity&>(refEntity);
}

