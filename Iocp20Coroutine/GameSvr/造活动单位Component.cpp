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

void 造活动单位Component::AddComponent(Entity& refEntity, PlayerGateSession_Game& refGateSession, const 建筑单位类型 类型)
{
	refEntity.m_sp造活动单位 = std::make_shared<造活动单位Component, PlayerGateSession_Game&, Entity&, const 建筑单位类型 >(refGateSession, refEntity, std::forward<const 建筑单位类型&&>(类型));
}

造活动单位Component::造活动单位Component(PlayerGateSession_Game& refSession, Entity& refEntity, const 建筑单位类型 类型) :m_refEntity(refEntity)
{
	switch (类型)
	{
	case 基地:m_set可造类型.insert(工程车); break;
	case 兵厂:
		m_set可造类型.insert(兵);
		m_set可造类型.insert(近战兵);
		break;
	default:
		break;
	}
}

void 造活动单位Component::造兵(PlayerGateSession_Game& refGateSession, Entity& refEntity, const 活动单位类型 类型)
{
	//CHECK_VOID(m_fun造活动单位);
	if (refGateSession.活动单位包括制造队列中的() >= refGateSession.活动单位上限())
	{
		refGateSession.播放声音("tadErr02", "民房不足"); //Additional supply depots required.需要更多的食堂
		return;
	}
	if (m_set可造类型.end() == m_set可造类型.find(类型))
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
	m_TaskCancel造活动单位.TryRun(Co造活动单位(refGateSession));
}


void 造活动单位Component::TryCancel(Entity& refEntity)
{
	m_TaskCancel造活动单位.TryCancel();
}

CoTaskBool 造活动单位Component::Co造活动单位(PlayerGateSession_Game& refGateSession)
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
		using namespace std;
		const auto posBuilding = m_refEntity.m_Pos;
		Position pos = { posBuilding.x + std::rand() % 10, posBuilding.z + +std::rand() % 10 };
		{
			const auto ok = m_refEntity.m_refSpace.CrowdToolFindNerestPos(pos);
			assert(ok);
		}
		bool CrowdTool可站立(CrowdToolState & refCrowTool, const Position & refPos);
		CHECK_CO_RET_FALSE(!refGateSession.m_wpSpace.expired());
		auto& refCrowdToolState = *refGateSession.m_wpSpace.lock()->m_spCrowdToolState;

		if (!CrowdTool可站立(refCrowdToolState, pos))
		{
			refGateSession.播放声音("此处不可放置");
			if (co_await CoTimer::Wait(1s, m_TaskCancel造活动单位.cancel))
			{
				m_list等待造.clear();
				co_return{};
			}
			continue;
		}

		if (配置.制造.u16消耗燃气矿 > refGateSession.m_u32燃气矿)
		{
			//std::ostringstream oss;
			refGateSession.播放声音("语音/燃气矿不足正经版", "燃气矿不足");//(low error beep) Insufficient Vespene Gas.气矿不足 
			//Say系统(oss.str());
			m_list等待造.clear();
			co_return{};
		}
		refGateSession.m_u32燃气矿 -= 配置.制造.u16消耗燃气矿;

		//先扣钱
		const auto& [stop, responce] = co_await AiCo::ChangeMoney(refGateSession, 配置.制造.u16消耗晶体矿, false, m_TaskCancel造活动单位.cancel);
		if (stop)
		{
			LOG(WARNING) << "协程RPC打断,error=" << responce.error << ",finalMoney=" << responce.finalMoney << ",rpcSn=" << responce.msg.rpcSnId;
			m_list等待造.clear();
			co_return{};
		}
		if (0 != responce.error)
		{
			refGateSession.m_u32燃气矿 += 配置.制造.u16消耗燃气矿;
			refGateSession.播放声音("语音/晶体矿不足正经版", "晶体矿矿不足");//Say系统("晶体矿矿不足" + 配置.建造.u16消耗晶体矿);
			m_list等待造.clear();
			co_return{};
		}
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

		LOG(INFO) << "协程RPC返回,error=" << responce.error << ",finalMoney=" << responce.finalMoney;
		CHECK_CO_RET_0(!refGateSession.m_wpSpace.expired());
		SpEntity spNewEntity = 造活动单位(refGateSession, pos, 配置, 类型);

		//if (m_list等待造.empty())
		//{
		//	LOG(ERROR) << "err";
		//	assert(false);
		//	co_return{};
		//}
	}

	EntitySystem::BroadcastEntity描述(m_refEntity, "造完了");
}

SpEntity 造活动单位Component::造活动单位(PlayerGateSession_Game& refGateSession, const Position& pos, const 单位::活动单位配置& 配置, const 活动单位类型 类型)
{
	auto sp = refGateSession.m_wpSpace.lock();
	SpEntity spNewEntity = std::make_shared<Entity, const Position&, Space&, const std::string&, const std::string&>(
		pos, *sp, 配置.配置.strPrefabName, 配置.配置.strName);
	spNewEntity->m_f警戒距离 = 配置.战斗.f警戒距离;
	PlayerComponent::AddComponent(*spNewEntity, refGateSession);
	AttackComponent::AddComponent(*spNewEntity, 类型, 配置.战斗.f攻击距离, 配置.战斗.f伤害);
	DefenceComponent::AddComponent(*spNewEntity, 配置.制造.u16初始Hp);
	走Component::AddComponent(*spNewEntity);
	refGateSession.m_setSpEntity.insert(spNewEntity);//自己控制的单位
	sp->m_mapEntity.insert({ (int64_t)spNewEntity.get() ,spNewEntity });//全地图单位

	switch (类型)
	{
	case 工程车:
		采集Component::AddComponent(*spNewEntity);
		造建筑Component::AddComponent(*spNewEntity, refGateSession, 类型);
		refGateSession.播放声音("TSCRdy00");//refGateSession.Say语音提示("工程车可以开工了!");//SCV, good to go, sir. SCV可以开工了
		break;
	case 兵:
		refGateSession.播放声音("TMaRdy00");//refGateSession.Say语音提示("听说有人要买我的狗头？");//You want a piece of me, boy?想要我的一部分吗，小子？
		break;
	case 近战兵:
		refGateSession.播放声音("tfbRdy00");//refGateSession.Say语音提示("听说有人要我的斧头？");//You want a piece of me, boy?想要我的一部分吗，小子？
		break;

	default:break;
	}

	spNewEntity->BroadcastEnter();
	refGateSession.Send资源();

	return spNewEntity;
}

uint16_t 造活动单位Component::等待造Count()const
{
	return (uint16_t)m_list等待造.size();
}