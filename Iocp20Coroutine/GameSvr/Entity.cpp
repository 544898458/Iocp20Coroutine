#include "pch.h"
#include "Entity.h"
#include "GameSvrSession.h"
#include "Space.h"
#include "../CoRoutine/CoTimer.h"
#include "../CoRoutine/CoEvent.h"
#include "GameSvr.h"
#include "AiCo.h"
#include <cmath>
#include <fstream>
#include "AiCo.h"
#include "PlayerGateSession_Game.h"
#include "../IocpNetwork/StrConv.h"
#include "EntitySystem.h"
#include "../IocpNetwork/MsgPack.h"
#include "单位组件/PlayerComponent.h"
#include "单位组件/MonsterComponent.h"
#include "单位组件/AttackComponent.h"
#include "单位组件/BuildingComponent.h"
#include "单位组件/采集Component.h"
#include "单位组件/造活动单位Component.h"
#include "单位组件/DefenceComponent.h"
#include "单位组件/地堡Component.h"
#include "单位组件/走Component.h"
#include "单位组件/造建筑Component.h"
#include "单位组件/AoiComponent.h"
#include "单位组件/飞向目标Component.h"
#include "单位组件/虫巢Component.h"
#include "单位组件/解锁单位Component.h"
#include "单位组件/升级单位属性Component.h"
#include "单位组件/PlayerNickNameComponent.h"
#include "单位组件/医疗兵Component.h"
#include "单位组件/找目标走过去Component.h"
#include "单位组件/BuffComponent.h"
#include "单位组件/苔蔓扩张Component.h"
#include "单位组件/苔蔓Component.h"
#include "单位组件/无苔蔓就持续掉血Component.h"
#include "单位组件/太岁Component.h"
#include "单位组件/临时阻挡Component.h"
#include "单位组件/资源Component.h"
#include "单位组件/数值Component.h"
#include "枚举/属性类型.h"


using namespace std;

Entity::Entity(const Position& pos, Space& space, const 单位类型 类型, const 单位::单位配置& ref配置) :
	Id((uint64_t)this), m_配置(ref配置), m_refSpace(space), m_Pos(pos),
	m_类型(类型)
{
}

bool Entity::DistanceLessEqual(const Entity& refEntity, float fDistance)
{
	return this->Pos().DistanceLessEqual(refEntity.Pos(), fDistance);
}

float Entity::DistancePow2(const Entity& refEntity)const
{
	return this->Pos().DistancePow2(refEntity.Pos());
}
float Entity::Distance(const Entity& refEntity)const
{
	return std::sqrtf(DistancePow2(refEntity));
}

bool Entity::IsDead() const
{
	if (!m_upDefence)
		return false;
	return m_upDefence->IsDead();
}

Entity::~Entity()
{
	//LOG(INFO) << "~Entity()," << Id;
}

struct SaveEntity
{
	单位类型 m_类型;
	单位::单位配置 m_配置;
	Position m_Pos;
	std::string m_strNickName;
	MSGPACK_DEFINE(m_类型, m_配置, m_Pos, m_strNickName);
};

void Entity::Save(std::ofstream& refOf)
{
	SaveEntity save = { m_类型,m_配置, m_Pos, EntitySystem::GetNickName(*this) };
	MsgPack::SendMsgpack(save, [&refOf](const void* buf, int len) { refOf.write((const char*)buf, len); }, true);
}

bool Entity::Load(Space& refSpace, char(&buf)[1024], const uint16_t u16Size)
{
	SaveEntity load;
	CHECK_FALSE(MsgPack::RecvMsgpack(load, buf, u16Size));

	switch (load.m_类型)
	{
	case 光刺:
		return false;
	default:
		break;
	}

	WpEntity wpNew;
	if (EntitySystem::Is建筑(load.m_类型))
	{
		SpEntity spNewEntity = std::make_shared<Entity, const Position&, Space&, 单位类型, const 单位::单位配置&>(
			load.m_Pos, refSpace, std::forward<单位类型&&>(load.m_类型), load.m_配置);
		PlayerComponent::AddComponent(*spNewEntity, {}, load.m_strNickName);
		造建筑Component::根据建筑类型AddComponent(refSpace, load.m_类型, *spNewEntity, {}, load.m_strNickName);

		CHECK_NOTNULL_RET_FALSE(spNewEntity->m_upBuilding);
		wpNew = spNewEntity;
	}
	else if (EntitySystem::Is活动单位(load.m_类型)) {
		单位::活动单位配置 活动单位配置;
		CHECK_FALSE(单位::Find活动单位配置(load.m_类型, 活动单位配置));

		wpNew = refSpace.造活动单位({}, load.m_strNickName, load.m_Pos, load.m_类型);
	}
	else
	{
		_ASSERT(false);
		return false;
	}

	CHECK_WP_RET_FALSE(wpNew);
	wpNew.lock()->OnLoad();
	return true;
}

//主线程单线程执行
void Entity::Update()
{
}

bool Entity::IsEnemy(const Entity& refEntity)
{
	if (!m_upPlayerNickName && !refEntity.m_upPlayerNickName)
		return false;//都是怪

	if (!m_upPlayerNickName || !refEntity.m_upPlayerNickName)
		return true;//有一个是怪

	//都是玩家单位
	return m_upPlayerNickName->m_strNickName != refEntity.m_upPlayerNickName->m_strNickName;
}

void Entity::SetPos(const Position& refNewPos)
{
	if (m_Pos == refNewPos)
		return;

	if (m_up临时阻挡)
		m_up临时阻挡.reset();//移动了，肯定要删除阻挡

	if (m_upAoi)
	{
		m_upAoi->OnBeforeChangePos(refNewPos);
	}
	m_Pos = refNewPos;
	if (m_upAoi)
	{
		m_upAoi->OnAfterChangePos();
	}
}

float Entity::攻击距离() const
{
	if (m_up找目标走过去)
		return m_up找目标走过去->m_战斗配置.f攻击距离;

	return 0;
}
float Entity::警戒距离() const
{
	if (m_up找目标走过去)
		return m_up找目标走过去->m_战斗配置.f警戒距离;

	return 0;
}

void Entity::OnLoad()
{
	if (m_upBuilding)
		m_upBuilding->直接造好();

	if (m_up虫巢)
		m_up虫巢->OnLoad();
}

void Entity::OnDestroy()
{
	BroadcastLeave();

	if (EntitySystem::Is视口(*this))
	{
		const auto sizeCount = m_refSpace.m_map视口.erase(Id);
		LOG_IF(ERROR, 1 != sizeCount) << "";
		_ASSERT(1 == sizeCount);
	}

	走Component::Cancel所有包含走路的协程(*this, true);

	for (auto& [id, proComponent] : m_mapComponentOnEntityDstroy)
	{
		proComponent->OnEntityDestroy(true);
	}

	if (m_cancelDelete)
	{
		LOG(INFO) << "取消删除协程";
		m_cancelDelete();
		m_cancelDelete = nullptr;
	}
	else
	{
		//LOG(INFO) << "没有删除协程";
	}
}

void Entity::BroadcastLeave()
{
	//LOG(INFO) << 头顶Name() << "调用Entity::BroadcastLeave," << Id;
	Broadcast(MsgDelRoleRet(Id));
}

const std::string& Entity::头顶Name()const
{
	if (m_upPlayerNickName)
		return m_upPlayerNickName->m_strNickName;

	if (m_up资源)
	{
		static const std::string str("资源");
		return str;
	}

	if (特效 == m_类型)
	{
		static const std::string str("");
		return str;
	}

	{
		static const std::string str("敌人");
		return str;
	}
}


void Entity::BroadcastEnter()
{
	//LOG(INFO) << NickName() << "调用Entity::BroadcastEnter," << Id;
	Broadcast(MsgAddRoleRet(*this));//自己广播给别人
	BroadcastNotifyPos();
	BroadcastNotify所有属性();

	if (苔蔓 == m_类型)
		EntitySystem::BroadcastEntity苔蔓半径(*this);

	CoEvent<MyEvent::AddEntity>::OnRecvEvent({ weak_from_this() });
}

void Entity::BroadcastNotifyPos()
{
	Broadcast(MsgNotifyPos(*this));
}


void Entity::BroadcastNotify属性(std::initializer_list<const 属性类型> list)
{
	MsgNotify属性 msg(*this,list);
	Broadcast(msg);
}


void Entity::BroadcastNotify所有属性()
{
	Broadcast(MsgNotify属性(*this, g_list所有属性));
}


void Entity::BroadcastChangeSkeleAnim(const std::string& refAniClipName, bool loop)
{
	Broadcast(MsgChangeSkeleAnim(*this, StrConv::GbkToUtf8(refAniClipName), loop));//播放死亡动作
}

template<class T>
void Entity::Broadcast(const T& msg)
{
	if (!m_upAoi)
		return;

	auto wp = m_wpOwner;
	if (wp.expired())
		wp = weak_from_this();

	CHECK_WP_RET_VOID(wp);

	auto& ref自己或Owner = *wp.lock();
	for (auto [k, wp能看到我] : ref自己或Owner.m_upAoi->m_map能看到我的)
	{
		CHECK_WP_CONTINUE(wp能看到我);
		auto& refEntity = *wp能看到我.lock();
		if (!EntitySystem::Is视口(refEntity))
			continue;

		if (refEntity.m_upPlayer)
			refEntity.m_upPlayer->m_refSession.Send(msg);
	}

	//m_refSpace.Broadcast(msg);
}

void Entity::DelayDelete(const std::chrono::system_clock::duration& dura)
{
	CoDelayDelete(dura).RunNew();
}
CoTaskBool Entity::CoDelayDelete(const std::chrono::system_clock::duration& dura)
{
	//LOG(INFO) << "开始延时删除自己的协程";
	//_ASSERT(!m_cancelDelete);//不可并行
	if (m_cancelDelete)
		co_return false;

	if (m_up地堡)
		m_up地堡->OnBeforeDelayDelete();

	using namespace std;
	if (co_await CoTimer::Wait(dura, m_cancelDelete))//服务器主工作线程大循环，每次循环触发一次
	{
		LOG(INFO) << "WaitDelete协程取消了";
		co_return true;
	}

	m_bNeedDelete = true;
	co_return false;
}


WpEntity Entity::Get最近的Entity支持地堡中的单位(const FindType bFindEnemy, std::function<bool(const Entity&)> fun符合条件)
{
	WpEntity wp = m_wpOwner;
	if (!m_refSpace.GetEntity(Id).expired())
	{
		wp = weak_from_this();
	}
	if (wp.expired())
		return {};

	return wp.lock()->Get最近的Entity(bFindEnemy, fun符合条件);
}

WpEntity Entity::Get最近的Entity(const FindType bFindEnemy, const 单位类型 目标类型)
{
	return Get最近的Entity(Entity::所有, [目标类型](const Entity& ref) {return ref.m_类型 == 目标类型; });
}

WpEntity Entity::Get最近的Entity(const FindType findType)
{
	return Get最近的Entity(findType, [](const Entity&) {return true; });
}

WpEntity Entity::Get最近的Entity(const FindType bFindEnemy, std::function<bool(const Entity&)> fun符合条件)
{
	if (!m_upAoi)
		return{};

	std::vector<std::pair<int64_t, WpEntity>> vecEnemy;
	std::copy_if(m_upAoi->m_map我能看到的.begin(), m_upAoi->m_map我能看到的.end(), std::back_inserter(vecEnemy),
		[bFindEnemy, this, &fun符合条件](const auto& pair)
		{
			auto& wp = pair.second;
			CHECK_FALSE(!wp.expired());
			Entity& ref = *wp.lock();
			if (ref.m_cancelDelete)
				return false;

			const auto bEnemy = ref.IsEnemy(*this);
			if (bFindEnemy == 敌方 && !bEnemy)
				return false;
			if (bFindEnemy == 友方 && bEnemy)
				return false;

			if (fun符合条件 && !fun符合条件(ref))
				return false;

			return &ref != this && !ref.IsDead();
		});

	if (vecEnemy.empty())
	{
		return {};
	}

	auto iterMin = std::min_element(vecEnemy.begin(), vecEnemy.end(), [this](const auto& pair1, const auto& pair2)
		{
			auto& sp1 = pair1.second;
			auto& sp2 = pair2.second;
			return this->DistancePow2(*sp1.lock()) < this->DistancePow2(*sp2.lock());
		});
	return iterMin->second.lock()->weak_from_this();
}
template void Entity::Broadcast(const MsgAddRoleRet& msg);
template void Entity::Broadcast(const MsgDelRoleRet& msg);
template void Entity::Broadcast(const MsgEntity描述& msg);
template void Entity::Broadcast(const Msg播放声音& msg);
template void Entity::Broadcast(const Msg弹丸特效& msg);
template void Entity::Broadcast(const MsgSay& msg);
template void Entity::Broadcast(const Msg苔蔓半径& msg);