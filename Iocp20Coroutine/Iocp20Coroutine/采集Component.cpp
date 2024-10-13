#include "pch.h"
#include "采集Component.h"
#include "BuildingComponent.h"
#include "Entity.h"
#include "../CoRoutine/CoTimer.h"
#include "Space.h"
#include "AiCo.h"

void 采集Component::采集(PlayerGateSession_Game& refGateSession, Entity& refThis, WpEntity wp目标资源)
{
	Co采集(refGateSession, refThis, wp目标资源).RunNew();
}

CoTaskBool 采集Component::Co采集(PlayerGateSession_Game& refGateSession, Entity& refThis, WpEntity wp目标资源)
{
	using namespace std;
	while (true)
	{
		if (wp目标资源.expired())
		{
			co_return 0;
		}

		auto wpEntity基地 = refThis.m_refSpace.Get最近的Entity(refThis, false, [](const Entity& ref)//找离自己最近的基地
			{
				if (!ref.m_spBuilding)
					return false;

				return ref.m_spBuilding->m_类型 == 基地;
			});

		if (wpEntity基地.expired())
		{
			co_await CoTimer::Wait(1s, m_funCancel);//没有基地，等一会儿再试
			continue;
		}

		if (Max携带晶体矿() <= m_u32携带晶体矿)//回基地放矿
		{
			//晶体矿已满
			if (refThis.DistanceLessEqual(*wpEntity基地.lock(), 3))//在基地附近，满载矿，全部放进基地（直接加钱）
			{
				const auto& [stop, _] = co_await AiCo::ChangeMoney(refGateSession, m_u32携带晶体矿, true, m_funCancel);
				if (stop)
				{
					co_return 0;
				}

				m_u32携带晶体矿 = 0;
				if(co_await CoTimer::Wait(1s, m_funCancel))//采矿速度
				{
					co_return 0;
				}
				continue;
			}
			else
			{
				//走向基地
				if (co_await AiCo::WalkToTarget(refThis.shared_from_this(), wpEntity基地.lock(), m_funCancel))
				{
					co_return 0;
				}
				continue;
			}
		}
		else//还要继续去采矿
		{
			if (refThis.DistanceLessEqual(*wp目标资源.lock(), 3))
			{
				++m_u32携带晶体矿;
				co_await CoTimer::Wait(1s, m_funCancel);//采矿速度
				continue;
			}
			else
			{
				//走向晶体矿
				co_await AiCo::WalkToTarget(refThis.shared_from_this(), wp目标资源.lock(), m_funCancel);
				continue;
			}
		}
	}
}

void 采集Component::AddComponent(Entity& refEntity)
{
	refEntity.m_sp采集 = std::make_shared<采集Component>();
}
