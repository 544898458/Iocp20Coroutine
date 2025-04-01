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
#include "AoiComponent.h"

void 造活动单位Component::AddComponent(Entity& refEntity)
{
	refEntity.m_sp造活动单位 = std::make_shared<造活动单位Component, Entity& >(refEntity);
}

造活动单位Component::造活动单位Component(Entity& refEntity) :m_refEntity(refEntity)
{
	switch (refEntity.m_类型)
	{
	case 基地:m_set可造类型.insert(工程车); break;
	case 机场:m_set可造类型.insert(飞机); break;
	case 兵营:
		m_set可造类型.insert(枪兵);
		m_set可造类型.insert(近战兵);
		break;
	case 重工厂:
		m_set可造类型.insert(三色坦克);
		break;
	case 幼虫:
		m_set可造类型.insert(工虫);
		m_set可造类型.insert(近战虫);
		m_set可造类型.insert(枪虫);
		m_set可造类型.insert(绿色坦克);
		m_set可造类型.insert(飞虫);
		m_set可造类型.insert(房虫);
		break;
	default:
		break;
	}
	m_pos集结点 = refEntity.Pos();
}

bool 造活动单位Component::可造(const 单位类型 类型)
{
	return m_set可造类型.end() != m_set可造类型.find(类型) && !Is幼虫正在蜕变();
}

void 造活动单位Component::造兵(PlayerGateSession_Game& refGateSession, const 单位类型 类型)
{
	//CHECK_VOID(m_fun造活动单位);
	if (房虫 != 类型 && refGateSession.活动单位包括制造队列中的() >= refGateSession.活动单位上限())
	{
		//refGateSession.播放声音("tadErr02", "民房不足"); //Additional supply depots required.需要更多的食堂
		switch (类型)
		{
		case 工虫:
		case 飞虫:
		case 近战虫:
		case 枪虫:
			refGateSession.播放声音("语音/房虫不足_男声", "房虫不足");
			break;
		default:
			refGateSession.播放声音("语音/民房不足女声正经版", "民房不足");
			break;
		}
		
		return;
	}
	if (!可造(类型))
	{
		refGateSession.Say系统("造不了这种单位");
		return;
	}

	switch (类型)
	{
	case 枪虫:
	case 近战虫:
		if (0 >= m_refEntity.m_refSpace.Get玩家单位数(refGateSession.NickName(), 虫营))
		{
			refGateSession.播放声音Buzz("缺少 虫营");
			return;
		}
		break;
	case 飞虫:
		if (0 >= m_refEntity.m_refSpace.Get玩家单位数(refGateSession.NickName(), 飞塔))
		{
			refGateSession.播放声音Buzz("缺少 飞塔");
			return;
		}
		break;
	default:
		break;
	}

	if (m_refEntity.m_spBuilding && !m_refEntity.m_spBuilding->已造好())
	{
		refGateSession.Say系统("建筑还没造好");
		return;
	}

	if (Is幼虫正在蜕变())
	{
		refGateSession.播放声音Buzz("幼虫正在蜕变");
		return;
	}

	m_list等待造.emplace_back(类型);//++m_i等待造兵数;
	m_TaskCancel造活动单位.TryRun(Co造活动单位());
}

bool 造活动单位Component::Is幼虫()const
{
	return 幼虫 == m_refEntity.m_类型;
}

bool 造活动单位Component::Is幼虫正在蜕变()
{
	return !m_TaskCancel造活动单位.co.Finished() && Is幼虫();
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
		单位::活动单位配置 活动;
		单位::单位配置 单位;
		单位::制造配置 制造;
		CHECK_CO_RET_FALSE(单位::Find活动单位配置(类型, 活动));
		CHECK_CO_RET_FALSE(单位::Find单位配置(类型, 单位));
		CHECK_CO_RET_FALSE(单位::Find制造配置(类型, 制造));
		auto& refSpace = m_refEntity.m_refSpace;
		using namespace std;
		const auto posBuilding = m_refEntity.Pos();
		Position pos = { posBuilding.x - 5 + std::rand() % 10, posBuilding.z - 5 + std::rand() % 10 };
		{
			const auto ok = refSpace.CrowdToolFindNerestPos(pos);
			_ASSERT(ok);
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

		if (制造.u16消耗燃气矿 > Space::GetSpacePlayer(m_refEntity).m_u32燃气矿)
		{
			//std::ostringstream oss;
			PlayerComponent::播放声音(m_refEntity, "语音/燃气矿不足正经版", "燃气矿不足");//(low error beep) Insufficient Vespene Gas.气矿不足 
			//Say系统(oss.str());
			m_list等待造.clear();
			co_return{};
		}
		refSpace.GetSpacePlayer(m_refEntity).m_u32燃气矿 -= 制造.u16消耗燃气矿;

		//先扣钱
		//const auto& [stop, responce] = co_await AiCo::ChangeMoney(refGateSession, 配置.制造.u16消耗晶体矿, false, m_TaskCancel造活动单位.cancel);
		//if (stop)
		//{
		//	LOG(WARNING) << "协程RPC打断,error=" << responce.error << ",finalMoney=" << responce.finalMoney << ",rpcSn=" << responce.msg.rpcSnId;
		//	m_list等待造.clear();
		//	co_return{};
		//}
		//if (0 != responce.error)
		if (制造.u16消耗晶体矿 > Space::GetSpacePlayer(m_refEntity).m_u32晶体矿)
		{
			//refGateSession.m_u32燃气矿 += 配置.制造.u16消耗燃气矿;
			PlayerComponent::播放声音(m_refEntity, "语音/晶体矿不足正经版", "晶体矿矿不足");//Say系统("晶体矿矿不足" + 配置.建造.u16消耗晶体矿);
			m_list等待造.clear();
			co_return{};
		}
		Space::GetSpacePlayer(m_refEntity).m_u32晶体矿 -= 制造.u16消耗晶体矿;
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
			if (Is幼虫())
				EntitySystem::BroadcastEntity描述(m_refEntity, std::format("蜕变进度{0}/{1}", i, MAX进度));
			else
				EntitySystem::BroadcastEntity描述(m_refEntity, std::format("待造队列{0},当前单位进度{1}/{2}", m_list等待造.size(), i, MAX进度));
		}

		//LOG(INFO) << "协程RPC返回,error=" << responce.error << ",finalMoney=" << responce.finalMoney;
		//CHECK_CO_RET_0(!refGateSession.m_wpSpace.expired());
		auto wpNewEntity = m_refEntity.m_refSpace.造活动单位(m_refEntity.m_spPlayer, EntitySystem::GetNickName(m_refEntity), pos, 类型);
		CHECK_WP_CO_RET_FALSE(wpNewEntity);
		auto& ref资源 = *wpNewEntity.lock();
		//if (m_list等待造.empty())
		//{
		//	LOG(ERROR) << "err";
		//	_ASSERT(false);
		//	co_return{};
		//}
		if (m_pos集结点 != m_refEntity.Pos())
		{
			if (!采集集结点附近的资源(ref资源))
			{
				CHECK_CO_RET_FALSE(ref资源.m_sp走);
				auto pos = m_pos集结点;
				m_refEntity.m_refSpace.CrowdToolFindNerestPos(pos);
				ref资源.m_sp走->WalkToPos(pos);
			}
		}
	}

	using namespace std;
	if (幼虫 == m_refEntity.m_类型)
		m_refEntity.CoDelayDelete(1ms).RunNew();
	else
		EntitySystem::BroadcastEntity描述(m_refEntity, "造完了");
}

bool 造活动单位Component::采集集结点附近的资源(Entity& refEntiy)const
{
	if (!refEntiy.m_sp采集)
		return false;//不是 工程车 或 工蜂

	const auto [i32格子Id, i32格子X, i32格子Z] = AoiComponent::格子(m_pos集结点);
	const auto& mapEntity = m_refEntity.m_refSpace.m_map在这一格里[i32格子Id];
	const auto iterFind = std::find_if(mapEntity.begin(), mapEntity.end(), [](const auto& pair)
		{
			CHECK_WP_RET_FALSE(pair.second);
			const auto& refEntity = *pair.second.lock();
			return EntitySystem::Is资源(refEntity.m_类型);
		});
	if (mapEntity.end() == iterFind)
		return false;

	CHECK_WP_RET_FALSE(iterFind->second);
	const auto& wp资源 = iterFind->second;
	if (!wp资源.lock()->Pos().DistanceLessEqual(m_pos集结点, 1))
		return false;

	refEntiy.m_sp采集->采集(wp资源);
	return true;
}

uint16_t 造活动单位Component::等待造Count()const
{
	return (uint16_t)m_list等待造.size();
}