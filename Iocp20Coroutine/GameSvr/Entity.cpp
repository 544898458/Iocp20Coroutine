#include "pch.h"
#include "Entity.h"
#include "GameSvrSession.h"
#include "Space.h"
#include "../CoRoutine/CoTimer.h"
#include "../CoRoutine/CoEvent.h"
#include "GameSvr.h"
#include "AiCo.h"
#include <cmath>
#include "单位组件/PlayerComponent.h"
#include "AiCo.h"
#include "../IocpNetwork/StrConv.h"
#include "单位组件/MonsterComponent.h"
#include "单位组件/AttackComponent.h"
#include "单位组件/BuildingComponent.h"
#include "PlayerGateSession_Game.h"
#include "单位组件/采集Component.h"
#include "单位组件/造活动单位Component.h"
#include "单位组件/DefenceComponent.h"
#include "单位组件/地堡Component.h"
#include "单位组件/走Component.h"
#include "单位组件/造建筑Component.h"
#include "单位组件/AoiComponent.h"
#include "EntitySystem.h"
#include "单位组件/PlayerNickNameComponent.h"
#include "../IocpNetwork/MsgPack.h"
#include <fstream>

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
	if (!m_spDefence)
		return false;
	return m_spDefence->IsDead();
}

Entity::~Entity()
{
	LOG(INFO) << "~Entity()," << Id;
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
	
	if (EntitySystem::Is建筑(load.m_类型))
	{
		SpEntity spNewEntity = std::make_shared<Entity, const Position&, Space&, 单位类型, const 单位::单位配置&>(
			load.m_Pos, refSpace, std::forward<单位类型&&>(load.m_类型), load.m_配置);
		PlayerComponent::AddComponent(*spNewEntity, {}, load.m_strNickName);
		单位::建筑单位配置 建筑配置;
		CHECK_FALSE(单位::Find建筑单位配置(load.m_类型, 建筑配置));
		造建筑Component::根据建筑类型AddComponent(refSpace, load.m_类型, *spNewEntity, {}, load.m_strNickName, 建筑配置);

		CHECK_NOTNULL_RET_FALSE(spNewEntity->m_spBuilding);
		spNewEntity->m_spBuilding->m_n建造进度百分比 = MAX建造百分比;
	}
	else if(EntitySystem::Is活动单位(load.m_类型)) {
		std::shared_ptr<PlayerComponent> spNull;
		单位::活动单位配置 活动单位配置;
		CHECK_FALSE(单位::Find活动单位配置(load.m_类型, 活动单位配置));

		refSpace.造活动单位(spNull, load.m_strNickName, load.m_Pos, 活动单位配置, load.m_类型);
	}

	return true;
}

//主线程单线程执行
void Entity::Update()
{
	if (m_spAttack)
		m_spAttack->Update();

	if (m_sp地堡)
		m_sp地堡->Update();
}

bool Entity::IsEnemy(const Entity& refEntity)
{
	if (!m_spPlayerNickName && !refEntity.m_spPlayerNickName)
		return false;//都是怪

	if (!m_spPlayerNickName || !refEntity.m_spPlayerNickName)
		return true;//有一个是怪

	//都是玩家单位
	return m_spPlayerNickName->m_strNickName != refEntity.m_spPlayerNickName->m_strNickName;
}

void Entity::SetPos(const Position& refNewPos)
{
	if (m_Pos == refNewPos)
		return;

	if (m_sp临时阻挡)
		m_sp临时阻挡.reset();//移动了，肯定要删除阻挡

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
	if (m_spAttack)
		return m_spAttack->m_战斗配置.f攻击距离;

	return 0;
}
float Entity::警戒距离() const
{
	if (m_spAttack)
		return m_spAttack->m_战斗配置.f警戒距离;

	return 0;
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
	//应该用proxy库同意调用下面的，免得忘了
	if (m_spAttack)
		m_spAttack->TryCancel(true);

	if (m_sp造活动单位)
		m_sp造活动单位->TryCancel(*this);

	if (m_sp采集)
		m_sp采集->m_TaskCancel.TryCancel();

	if (m_sp走)
		m_sp走->TryCancel();

	if (m_sp地堡)
		m_sp地堡->OnDestroy();

	if (m_sp临时阻挡)
		m_sp临时阻挡.reset();

	if (m_sp造建筑)
		m_sp造建筑->TryCancel();

	if (m_cancelDelete)
	{
		LOG(INFO) << "取消删除协程";
		m_cancelDelete();
	}
	else
	{
		LOG(INFO) << "没有删除协程";
	}

	if (m_upAoi)
		m_upAoi->OnDestory();
}

void Entity::BroadcastLeave()
{
	LOG(INFO) << 头顶Name() << "调用Entity::BroadcastLeave," << Id;
	Broadcast(MsgDelRoleRet(Id));
}

const std::string& Entity::头顶Name()const
{
	if (m_spPlayerNickName)
		return m_spPlayerNickName->m_strNickName;

	if (m_sp资源)
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
	CoEvent<MyEvent::AddEntity>::OnRecvEvent({ weak_from_this() });
}

void Entity::BroadcastNotifyPos()
{
	Broadcast(MsgNotifyPos(*this));
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

	auto& ref自己或Owner = *wp.lock();
	for (auto [k, wp能看到我] : ref自己或Owner.m_upAoi->m_map能看到我的)
	{
		CHECK_WP_CONTINUE(wp能看到我);
		auto& refEntity = *wp能看到我.lock();
		if (!EntitySystem::Is视口(refEntity))
			continue;

		if (refEntity.m_spPlayer)
			refEntity.m_spPlayer->m_refSession.Send(msg);
	}

	//m_refSpace.Broadcast(msg);
}

CoTaskBool Entity::CoDelayDelete(const std::chrono::system_clock::duration& dura)
{
	LOG(INFO) << "开始删除延时自己的协程";
	//_ASSERT(!m_cancelDelete);//不可并行
	if (m_cancelDelete)
		co_return false;

	if (m_sp地堡)
		m_sp地堡->OnBeforeDelayDelete();

	using namespace std;
	if (co_await CoTimer::Wait(dura, m_cancelDelete))//服务器主工作线程大循环，每次循环触发一次
	{
		LOG(INFO) << "WaitDelete协程取消了";
		co_return true;
	}

	m_bNeedDelete = true;
	co_return false;
}

template void Entity::Broadcast(const MsgAddRoleRet& msg);
template void Entity::Broadcast(const MsgDelRoleRet& msg);
template void Entity::Broadcast(const MsgEntity描述& msg);
template void Entity::Broadcast(const Msg播放声音& msg);
template void Entity::Broadcast(const Msg弹丸特效& msg);
template void Entity::Broadcast(const MsgSay& msg);