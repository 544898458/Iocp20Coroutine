#include "pch.h"
#include "造活动单位Component.h"
#include "PlayerGateSession_Game.h"
#include "采集Component.h"
#include "造建筑Component.h"
#include "单位.h"
#include "../CoRoutine/CoRpc.h"
#include "../CoRoutine/CoTimer.h"
#include "../枚举/单位类型.h"
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
	refEntity.AddComponentOnDestroy(&Entity::m_up造活动单位, refEntity);
}

造活动单位Component::造活动单位Component(Entity& refEntity) :m_refEntity(refEntity)
{
	switch (refEntity.m_类型)
	{
	case 基地:m_set可造类型 = { 工程车, 医疗兵 }; break;
	case 机场:m_set可造类型 = { 飞机 }; break;
	case 兵营:m_set可造类型 = { 枪兵,近战兵 }; break;
	case 重车厂:m_set可造类型 = { 三色坦克 }; break;
	case 幼虫:m_set可造类型 = { 工虫, 近战虫, 枪虫, 绿色坦克, 飞虫, 房虫 }; break;
	default:
		LOG(ERROR) << "未知的建造单位类型," << refEntity.m_类型;
		_ASSERT(!"未知的建造单位类型");
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
	bool b已达单玩家活动单位上限 = false;
	if (房虫 != 类型 && refGateSession.活动单位包括制造队列中的() >= refGateSession.活动单位上限(std::forward<bool&&>(b已达单玩家活动单位上限)))
	{
		if (b已达单玩家活动单位上限)
		{
			refGateSession.播放声音Buzz(std::format("已达到单个玩家活动单位上限"));
			return;
		}

		switch (类型)
		{
		case 工虫:
		case 飞虫:
		case 近战虫:
		case 枪虫:
		case 绿色坦克:
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

	if (!EntitySystem::判断前置单位存在(m_refEntity, 类型))
	{
		return;
	}

	auto& spacePlayer = m_refEntity.m_refSpace.GetSpacePlayer(refGateSession.NickName());
	if (!spacePlayer.已解锁(类型))
	{
		switch (类型)
		{
		case 近战兵:
			refGateSession.播放声音Buzz("请先在兵营中解锁");
			break;
		case 枪虫:
			refGateSession.播放声音Buzz("请先在虫营中解锁");
			break;
		default:
			LOG(ERROR) << "未知的未解锁单位类型," << 类型;
			break;
		}

		return;
	}

	if (m_refEntity.m_upBuilding && !m_refEntity.m_upBuilding->已造好())
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
	if(m_TaskCancel造活动单位.co.Finished())
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

void 造活动单位Component::OnEntityDestroy(const bool bDestroy)
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
		Position pos;
		if (Is幼虫())
		{
			pos = posBuilding;
			走Component::Cancel所有包含走路的协程(m_refEntity, true);
		}
		else
		{
			pos = { posBuilding.x - 5 + std::rand() % 10, 0, posBuilding.z - 5 + std::rand() % 10 };
			const auto ok = refSpace.CrowdToolFindNerestPos(pos);
			_ASSERT(ok);

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
		}

		if (制造.消耗.u16消耗燃气矿 > Space::GetSpacePlayer(m_refEntity).m_u32燃气矿)
		{
			//std::ostringstream oss;
			PlayerComponent::播放声音(m_refEntity, "语音/燃气矿不足正经版", "燃气矿不足");
			//Say系统(oss.str());
			m_list等待造.clear();
			co_return{};
		}
		refSpace.GetSpacePlayer(m_refEntity).m_u32燃气矿 -= 制造.消耗.u16消耗燃气矿;

		if (制造.消耗.u16消耗晶体矿 > Space::GetSpacePlayer(m_refEntity).m_u32晶体矿)
		{
			//refGateSession.m_u32燃气矿 += 配置.制造.u16消耗燃气矿;
			PlayerComponent::播放声音(m_refEntity, "语音/晶体矿不足正经版", "晶体矿不足");
			m_list等待造.clear();
			co_return{};
		}
		Space::GetSpacePlayer(m_refEntity).m_u32晶体矿 -= 制造.消耗.u16消耗晶体矿;

		if (Is幼虫())
			m_refEntity.BroadcastChangeSkeleAnim("Armature|Armature.003|Take 001|BaseLayer");

		const int MAX进度 = 10;
		for (int i = 0; i < 10; ++i)
		{
			if (co_await CoTimer::Wait(300ms, m_TaskCancel造活动单位.cancel))
			{
				m_list等待造.clear();
				co_return{};
			}
			LOG(INFO) << std::format("进度{0}/{1}", i, MAX进度);
			if (Is幼虫())
				EntitySystem::BroadcastEntity描述(m_refEntity, std::format("蜕变进度{0}/{1}", i, MAX进度));
			else
				EntitySystem::BroadcastEntity描述(m_refEntity, std::format("待造队列{0},当前单位进度{1}/{2}", m_list等待造.size(), i, MAX进度));
		}

		//LOG(INFO) << "协程RPC返回,error=" << responce.error << ",finalMoney=" << responce.finalMoney;
		//CHECK_CO_RET_0(!refGateSession.m_wpSpace.expired());
		auto wpNewEntity = m_refEntity.m_refSpace.造活动单位(std::forward<UpPlayerComponent>(m_refEntity.m_upPlayer), EntitySystem::GetNickName(m_refEntity), pos, 类型);
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
				CHECK_CO_RET_FALSE(ref资源.m_up走);
				auto pos = m_pos集结点;
				m_refEntity.m_refSpace.CrowdToolFindNerestPos(pos);
				ref资源.m_up走->WalkToPos(pos);
			}
		}
	}

	using namespace std;
	if (幼虫 == m_refEntity.m_类型)
		m_refEntity.DelayDelete(1ms);
	else
		EntitySystem::BroadcastEntity描述(m_refEntity, "造完了");
}

bool 造活动单位Component::采集集结点附近的资源(Entity& refEntiy)const
{
	if (!refEntiy.m_up采集)
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

	refEntiy.m_up采集->采集(wp资源);
	return true;
}

uint16_t 造活动单位Component::等待造Count()const
{
	return (uint16_t)m_list等待造.size();
}