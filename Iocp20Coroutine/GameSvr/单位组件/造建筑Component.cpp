#include "pch.h"
#include "造建筑Component.h"
#include "单位.h"
#include "AiCo.h"
#include "Entity.h"
#include "EntitySystem.h"
#include "PlayerComponent.h"
#include "AttackComponent.h"
#include "PlayerGateSession_Game.h"
#include "../CoRoutine/CoTimer.h"
#include "../IocpNetwork/StrConv.h"
#include "造活动单位Component.h"
#include "地堡Component.h"
#include "DefenceComponent.h"
#include "BuildingComponent.h"
#include "虫巢Component.h"
#include "解锁单位Component.h"
#include "升级单位属性Component.h"
#include "苔蔓Component.h"
#include "苔蔓扩张Component.h"
#include "苔蔓Component.h"
#include "走Component.h"
#include "AoiComponent.h"
#include "BuffComponent.h"
#include "无苔蔓就持续掉血Component.h"

void 造建筑Component::AddComponent(Entity& refEntity)
{
	refEntity.m_sp造建筑 = std::make_shared<造建筑Component, Entity&>(refEntity);
}

bool 造建筑Component::正在建造(const Entity& refEntity)
{
	if (!refEntity.m_sp造建筑)
		return false;

	return refEntity.m_sp造建筑->m_cancel造建筑.operator bool();
}

造建筑Component::造建筑Component(Entity& refEntity) :m_refEntity(refEntity)
{
	switch (refEntity.m_类型)
	{
	case 工程车:
		m_set可造类型 = { 基地,民房,兵营,地堡,炮台,机场,重车厂 };
		break;
	case 工虫:
		m_set可造类型 = { 虫巢,虫营,飞塔,拟态源,炮台,太岁 };
		break;
	default:
		break;
	}
}

造建筑Component::~造建筑Component()
{
	_ASSERT(!m_cancel造建筑.operator bool());
}

bool 造建筑Component::可造(const 单位类型 类型)const
{
	return m_set可造类型.end() != m_set可造类型.find(类型);
}

void 造建筑Component::造建筑(const Position pos, const 单位类型 类型)
{
	Co造建筑(pos, 类型).RunNew();
}

CoTaskBool 造建筑Component::Co造建筑(const Position pos, const 单位类型 类型)
{
	if (!可造(类型))
	{
		co_return false;
	}

	//先走到目标点
	单位::建筑单位配置 配置;
	if (!单位::Find建筑单位配置(类型, 配置))
	{
		_ASSERT(false);
		co_return true;
	}
	if (!m_refEntity.m_refSpace.可放置建筑(pos, 配置.f半边长))
	{
		//播放声音("TSCErr00", "有阻挡，无法建造");//（Err00） I can't build it, something's in the way. 我没法在这建，有东西挡道
		PlayerComponent::播放声音(m_refEntity, "语音/无法在这里建造可爱版", "有阻挡，无法建造");
		co_return true;
	}

	//只有虫类建筑单位可以造在苔蔓上，而且必须造在苔蔓上（虫巢除外）
	{
		bool b此处有苔蔓 = 此处有苔蔓吗(pos);
		if (单位::Is虫(类型))
		{
			if (虫巢 != 类型 && !b此处有苔蔓)
			{
				PlayerGateSession_Game::播放声音Buzz(m_refEntity, "请在有苔蔓的地方变异");
				co_return false;
			}
		}
		else
		{
			if (b此处有苔蔓)
			{
				PlayerGateSession_Game::播放声音Buzz(m_refEntity, "请在没有苔蔓的地方建造");
				co_return false;
			}
		}
	}

	PlayerComponent::播放声音(m_refEntity, "语音/明白女声可爱版");

	EntitySystem::BroadcastEntity描述(m_refEntity, "走向建造点");

	CHECK_CO_RET_FALSE(m_refEntity.m_sp走);
	if (co_await m_refEntity.m_sp走->WalkToPos(pos, m_cancel造建筑, 配置.f半边长 + 1))
		co_return true;

	//然后开始扣钱建造
	//CHECK_CO_RET_FALSE(m_refEntity.m_spPlayer);
	auto wpEntity建筑 = AddBuilding(类型, pos);
	if (wpEntity建筑.expired())
		co_return false;

	auto& ref建筑 = *wpEntity建筑.lock();
	CHECK_CO_RET_FALSE(ref建筑.m_spBuilding);

	if (工程车 == m_refEntity.m_类型)
	{
		EntitySystem::BroadcastEntity描述(m_refEntity, "正在建造");

		if (m_refEntity.m_类型 == 工程车)
			m_refEntity.BroadcastChangeSkeleAnim("2", true);

		if (co_await Co建造过程(wpEntity建筑, m_cancel造建筑))
			co_return true;

		EntitySystem::BroadcastEntity描述(m_refEntity, "");
	}
	else
	{
		using namespace std;
		m_refEntity.CoDelayDelete(1ms).RunNew();
		ref建筑.m_spBuilding->StartCo建造过程();
	}

	co_return false;
}

CoTaskBool 造建筑Component::Co建造过程(WpEntity wpEntity建筑, FunCancel& cancel)
{
	PlayerComponent::播放声音(m_refEntity, "音效/打桩机破碎镐");

	KeepCancel kc(cancel);
	std::weak_ptr<BuildingComponent> wpBuilding(wpEntity建筑.lock()->m_spBuilding);

	while (!wpBuilding.expired() && BuildingComponent::MAX建造百分比 > wpBuilding.lock()->m_n建造进度百分比)
	{
		if (co_await CoTimer::WaitNextUpdate(cancel))
			co_return true;
		if (wpBuilding.expired())
			co_return true;

		BuildingComponent& refBuilding = *wpBuilding.lock();

		if (refBuilding.m_n建造进度百分比 > 0 && 0 == refBuilding.m_n建造进度百分比 % 20)
			PlayerComponent::播放声音(m_refEntity, "EDrRep00");//Repair

		++refBuilding.m_n建造进度百分比;

		std::ostringstream oss;
		if (BuildingComponent::MAX建造百分比 <= refBuilding.m_n建造进度百分比)
			oss << "建造完成";
		else
			oss << "正在建造:" << refBuilding.m_n建造进度百分比 << "%";

		_ASSERT(!wpEntity建筑.expired());
		EntitySystem::BroadcastEntity描述(*wpEntity建筑.lock(), oss.str());
		PlayerComponent::Send资源(m_refEntity);
	}

	if (m_refEntity.m_spAttack)
	{
		switch (m_refEntity.m_类型)
		{
		case 工程车:PlayerComponent::播放声音(m_refEntity, "语音/建造完成女声可爱版"); break;
		default:break;
		}
	}
	co_return 0;
}

void 造建筑Component::TryCancel()
{
	if (m_cancel造建筑)
	{
		//LOG(INFO) << "调用m_cancel";
		m_cancel造建筑();
	}
	else
	{

	}
}


WpEntity 造建筑Component::AddBuilding(const 单位类型 类型, const Position pos)
{
	单位::建筑单位配置 建筑;
	单位::单位配置 单位;
	单位::制造配置 制造;
	CHECK_RET_DEFAULT(单位::Find建筑单位配置(类型, 建筑));
	CHECK_RET_DEFAULT(单位::Find单位配置(类型, 单位));
	CHECK_RET_DEFAULT(单位::Find制造配置(类型, 制造));

	//Position pos = { 35,float(std::rand() % 60) - 30 };
	if (!m_refEntity.m_refSpace.可放置建筑(pos, 建筑.f半边长))
	{
		//播放声音("TSCErr00", "有阻挡，无法建造");//（Err00） I can't build it, something's in the way. 我没法在这建，有东西挡道
		PlayerComponent::播放声音(m_refEntity, "语音/无法在这里建造可爱版", "有阻挡");
		return{};
	}
	if (制造.消耗.u16消耗燃气矿 > Space::GetSpacePlayer(m_refEntity).m_u32燃气矿)
	{
		//std::ostringstream oss;
		PlayerComponent::播放声音(m_refEntity, "语音/燃气矿不足可爱版", "燃气矿不足无法建造");// << 配置.建造.u16消耗燃气矿;//(low error beep) Insufficient Vespene Gas.气矿不足 
		//Say系统(oss.str());
		return{};
	}
	Space::GetSpacePlayer(m_refEntity).m_u32燃气矿 -= 制造.消耗.u16消耗燃气矿;
	//auto iterNew = m_vecFunCancel.insert(m_vecFunCancel.end(), std::make_shared<FunCancel>());//不能存对象，扩容可能导致引用和指针失效
	//auto [stop, responce] = co_await AiCo::ChangeMoney(*this, 配置.建造.u16消耗晶体矿, false, **iterNew);
	//LOG(INFO) << "协程RPC返回,error=" << responce.error << ",finalMoney=" << responce.finalMoney;
	//if (stop)
	//{
	//	m_u32燃气矿 += 配置.建造.u16消耗燃气矿;//返还燃气矿
	//	co_return{};
	//}
	//if (0 != responce.error)
	if (制造.消耗.u16消耗晶体矿 > Space::GetSpacePlayer(m_refEntity).m_u32晶体矿)
	{
		//LOG(WARNING) << "扣钱失败,error=" << responce.error;
		//m_u32燃气矿 += 配置.建造.u16消耗燃气矿;//返还燃气矿
		PlayerComponent::播放声音(m_refEntity, "语音/晶体矿不足可爱版", "晶体矿不足");

		return{};
	}
	Space::GetSpacePlayer(m_refEntity).m_u32晶体矿 -= 制造.消耗.u16消耗晶体矿;

	PlayerComponent::Send资源(m_refEntity);

	//加建筑
	//CHECK_CO_RET_0(!m_wpSpace.expired());
	//auto spSpace = m_wpSpace.lock();
	auto wpNew = 创建建筑(m_refEntity.m_refSpace, pos, 类型, m_refEntity.m_spPlayer, EntitySystem::GetNickName(m_refEntity));
	PlayerComponent::Send资源(m_refEntity);
	return wpNew;
}

bool 造建筑Component::此处有苔蔓吗(const Position pos)
{
	const auto [id格子, _, __] = AoiComponent::格子(pos);
	auto mapWp = m_refEntity.m_refSpace.m_map能看到这一格[id格子];
	for (const auto& [id, wp] : mapWp)
	{
		CHECK_WP_CONTINUE(wp);
		const auto& refEntity = *wp.lock();
		if (refEntity.m_up苔蔓 && refEntity.m_up苔蔓->在半径内(pos))
			return true;
	}
	return false;
}

WpEntity 造建筑Component::创建建筑(Space& refSpace, const Position& pos, const 单位类型 类型, SpPlayerComponent spPlayer, const std::string& strPlayerNickName)
{
	单位::建筑单位配置 建筑;
	单位::单位配置 单位;
	单位::制造配置 制造;
	CHECK_RET_DEFAULT(单位::Find建筑单位配置(类型, 建筑));
	CHECK_RET_DEFAULT(单位::Find单位配置(类型, 单位));
	CHECK_RET_DEFAULT(单位::Find制造配置(类型, 制造));

	auto spNewEntity = std::make_shared<Entity, const Position&, Space&, const 单位类型, const 单位::单位配置& >(
		pos, refSpace, std::forward<const 单位类型&&>(类型), 单位);
	//spNewEntity->AddComponentAttack();
	根据建筑类型AddComponent(refSpace, 类型, *spNewEntity, spPlayer, strPlayerNickName);

	spNewEntity->BroadcastEnter();
	return spNewEntity;
}

void 造建筑Component::根据建筑类型AddComponent(Space& refSpace, const 单位类型 类型, Entity& refNewEntity, SpPlayerComponent spPlayer, const std::string& strPlayerNickName)
{
	单位::建筑单位配置 建筑;
	单位::单位配置 单位;
	单位::制造配置 制造;
	CHECK_RET_VOID(单位::Find建筑单位配置(类型, 建筑));
	CHECK_RET_VOID(单位::Find单位配置(类型, 单位));
	CHECK_RET_VOID(单位::Find制造配置(类型, 制造));

	if (spPlayer)
		PlayerComponent::AddComponent(refNewEntity, spPlayer, spPlayer->m_refSession.NickName());

	BuildingComponent::AddComponent(refNewEntity, 建筑.f半边长);

	switch (类型)
	{
	case 虫营:
		解锁单位Component::AddComponent(refNewEntity);
		升级单位属性Component::AddComponent(refNewEntity);
		break;
	case 兵营:
		解锁单位Component::AddComponent(refNewEntity);
		造活动单位Component::AddComponent(refNewEntity);
		升级单位属性Component::AddComponent(refNewEntity);
		break;
	case 基地:
		造活动单位Component::AddComponent(refNewEntity);
		break;
	case 机场:
	case 重车厂:
		升级单位属性Component::AddComponent(refNewEntity);
		造活动单位Component::AddComponent(refNewEntity);
		break;
	case 飞塔:
		升级单位属性Component::AddComponent(refNewEntity);
		break;
	case 地堡:
		地堡Component::AddComponent(refNewEntity);
		break;
	case 民房:
		break;
	case 炮台:
		AttackComponent::AddComponent(refNewEntity);
		break;
	case 虫巢:
		虫巢Component::AddComponent(refNewEntity);
		苔蔓扩张Component::AddComponent(refNewEntity);
		break;
	case 太岁:
		苔蔓扩张Component::AddComponent(refNewEntity);
		break;
	case 苔蔓:
		苔蔓Component::AddComponent(refNewEntity);
		break;
	default:
		break;
	}
	if (苔蔓 != 类型)
	{
		DefenceComponent::AddComponent(refNewEntity, 制造.u16初始Hp);
		BuffComponent::AddComponent(refNewEntity);
	}
	if (单位::Is虫(类型) && EntitySystem::Is建筑(类型))
	{
        无苔蔓就持续掉血Component::AddComponent(refNewEntity);
	}

	refSpace.m_mapPlayer[strPlayerNickName].m_mapWpEntity[refNewEntity.Id] = refNewEntity.shared_from_this();//自己控制的单位
	const int i32视野范围 = 苔蔓 == 类型 ? 苔蔓Component::MAX半径 : 0;
	refSpace.AddEntity(refNewEntity.shared_from_this(), i32视野范围);

	if (虫巢 == 类型 || 太岁 == 类型)
	{
		auto wp苔蔓 = 造建筑Component::创建建筑(refSpace, refNewEntity.Pos(), 苔蔓, spPlayer, strPlayerNickName);
		CHECK_WP_RET_VOID(wp苔蔓);
		auto& ref苔蔓 = *wp苔蔓.lock();

		CHECK_RET_VOID(ref苔蔓.m_up苔蔓->m_wp附着建筑.expired());
		ref苔蔓.m_up苔蔓->m_wp附着建筑 = refNewEntity.shared_from_this();

		CHECK_RET_VOID(refNewEntity.m_up苔蔓扩张->m_wp苔蔓.expired());
		refNewEntity.m_up苔蔓扩张->m_wp苔蔓 = wp苔蔓;
	}
}
