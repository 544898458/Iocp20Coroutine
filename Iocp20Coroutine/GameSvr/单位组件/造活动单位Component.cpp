#include "pch.h"
#include "造活动单位Component.h"
#include "PlayerGateSession_Game.h"
#include "采集Component.h"
#include "造建筑Component.h"
#include "单位.h"
#include "../CoRoutine/CoRpc.h"
#include "../CoRoutine/CoTimer.h"
#include "AiCo.h"
#include "AttackComponent.h"
#include "DefenceComponent.h"
#include "PlayerComponent.h"
#include "走Component.h"
#include "EntitySystem.h"
#include "BuildingComponent.h"

void 造活动单位Component::AddComponent(Entity& refEntity, const 单位类型 类型)
{
	refEntity.m_sp造活动单位 = std::make_shared<造活动单位Component, Entity&, const 单位类型 >(refEntity, std::forward<const 单位类型&&>(类型));
}

造活动单位Component::造活动单位Component(Entity& refEntity, const 单位类型 类型) :m_refEntity(refEntity)
{
	switch (类型)
	{
	case 基地:m_set可造类型.insert(工程车); break;
	case 兵厂:
		m_set可造类型.insert(兵);
		m_set可造类型.insert(近战兵);
		m_set可造类型.insert(三色坦克);
		break;
	default:
		break;
	}
}

bool 造活动单位Component::可造(const 单位类型 类型)const
{
	return m_set可造类型.end() != m_set可造类型.find(类型);
}

void 造活动单位Component::造兵(PlayerGateSession_Game& refGateSession, const 单位类型 类型)
{
	//CHECK_VOID(m_fun造活动单位);
	if (refGateSession.活动单位包括制造队列中的() >= refGateSession.活动单位上限())
	{
		//refGateSession.播放声音("tadErr02", "民房不足"); //Additional supply depots required.需要更多的食堂
		refGateSession.播放声音("语音/民房不足女声正经版", "民房不足");
		return;
	}
	if (!可造(类型))
	{
		refGateSession.Say系统("造不了这种单位");
		return;
	}

	if (!m_refEntity.m_spBuilding->已造好())
	{
		refGateSession.Say系统("建筑还没造好");
		return;
	}

	m_list等待造.emplace_back(类型);//++m_i等待造兵数;
	m_TaskCancel造活动单位.TryRun(Co造活动单位());
}


void 造活动单位Component::TryCancel(Entity& refEntity)
{
	m_TaskCancel造活动单位.TryCancel();
}

CoTaskBool 造活动单位Component::Co造活动单位()
{
	while (!m_list等待造.empty())
	{
		const auto 类型(m_list等待造.front());
		m_list等待造.pop_front();//--refThis.m_i等待造兵数;
		单位::活动单位配置 配置;
		if (!单位::Find活动单位配置(类型, 配置))
		{
			co_return{};
		}
		auto &refSpace = m_refEntity.m_refSpace;
		using namespace std;
		const auto posBuilding = m_refEntity.Pos();
		Position pos = { posBuilding.x + std::rand() % 10, posBuilding.z + +std::rand() % 10 };
		{
			const auto ok = refSpace.CrowdToolFindNerestPos(pos);
			_ASSERTok);
		}
		
		if (!refSpace.CrowdTool可站立(pos))
		{
			PlayerComponent::播放声音(m_refEntity, "此处不可放置");
			if (co_await CoTimer::Wait(1s, m_TaskCancel造活动单位.cancel))
			{
				m_list等待造.clear();
				co_return{};
			}
			continue;
		}

		if (配置.制造.u16消耗燃气矿 > Space::GetSpacePlayer(m_refEntity).m_u32燃气矿)
		{
			//std::ostringstream oss;
			PlayerComponent::播放声音(m_refEntity, "语音/燃气矿不足正经版", "燃气矿不足");//(low error beep) Insufficient Vespene Gas.气矿不足 
			//Say系统(oss.str());
			m_list等待造.clear();
			co_return{};
		}
		refSpace.GetSpacePlayer(m_refEntity).m_u32燃气矿 -= 配置.制造.u16消耗燃气矿;

		//先扣钱
		//const auto& [stop, responce] = co_await AiCo::ChangeMoney(refGateSession, 配置.制造.u16消耗晶体矿, false, m_TaskCancel造活动单位.cancel);
		//if (stop)
		//{
		//	LOG(WARNING) << "协程RPC打断,error=" << responce.error << ",finalMoney=" << responce.finalMoney << ",rpcSn=" << responce.msg.rpcSnId;
		//	m_list等待造.clear();
		//	co_return{};
		//}
		//if (0 != responce.error)
		if (配置.制造.u16消耗晶体矿 > Space::GetSpacePlayer(m_refEntity).m_u32晶体矿)
		{
			//refGateSession.m_u32燃气矿 += 配置.制造.u16消耗燃气矿;
			PlayerComponent::播放声音(m_refEntity, "语音/晶体矿不足正经版", "晶体矿矿不足");//Say系统("晶体矿矿不足" + 配置.建造.u16消耗晶体矿);
			m_list等待造.clear();
			co_return{};
		}
		Space::GetSpacePlayer(m_refEntity).m_u32晶体矿-= 配置.制造.u16消耗晶体矿;
		//耗时
		//if (co_await CoTimer::Wait(1s, m_TaskCancel造活动单位.cancel))
		//{
		//	co_return{};
		//}
		const int MAX进度 = 10;
		for (int i = 0; i < 10; ++i)
		{
			if (co_await CoTimer::Wait(300ms, m_TaskCancel造活动单位.cancel))
			{
				m_list等待造.clear();
				co_return{};
			}
			EntitySystem::BroadcastEntity描述(m_refEntity, std::format("待造队列{0},当前单位进度{1}/{2}", m_list等待造.size(), i, MAX进度));
		}

		//LOG(INFO) << "协程RPC返回,error=" << responce.error << ",finalMoney=" << responce.finalMoney;
		//CHECK_CO_RET_0(!refGateSession.m_wpSpace.expired());
		SpEntity spNewEntity = m_refEntity.m_refSpace.造活动单位(m_refEntity.m_spPlayer, EntitySystem::GetNickName(m_refEntity), pos, 配置, 类型);

		//if (m_list等待造.empty())
		//{
		//	LOG(ERROR) << "err";
		//	_ASSERTfalse);
		//	co_return{};
		//}
	}

	EntitySystem::BroadcastEntity描述(m_refEntity, "造完了");
}

uint16_t 造活动单位Component::等待造Count()const
{
	return (uint16_t)m_list等待造.size();
}