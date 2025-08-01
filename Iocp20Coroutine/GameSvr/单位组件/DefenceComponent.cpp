#include "pch.h"
#include "DefenceComponent.h"
#include "Entity.h"
#include "EntitySystem.h"
#include "AiCo.h"
#include "../CoRoutine/CoEvent.h"
#include "MyEvent.h"
#include "AttackComponent.h"
#include "PlayerComponent.h"
#include "BuildingComponent.h"
#include "临时阻挡Component.h"
#include "数值Component.h"
#include "../Space.h"
#include "../枚举/属性类型.h"
#include "../../读配置文件/Try读Ini本地机器专用.h"


DefenceComponent::DefenceComponent(Entity& refEntity, const uint16_t i16HpMax) : m_refEntity(refEntity)
{
	数值Component::Set(refEntity, 属性类型::最大生命, i16HpMax);
	数值Component::Set(refEntity, 生命, i16HpMax);
}

void DefenceComponent::AddComponent(Entity& refEntity, uint16_t u16初始Hp)
{
	数值Component::AddComponent(refEntity);
	CHECK_RET_VOID(!refEntity.m_upDefence);
	refEntity.m_upDefence.reset(new DefenceComponent(refEntity, u16初始Hp));
}

uint16_t DefenceComponent::升级后的防御(Entity& refEntity)
{
	//uint16_t u16等级(0);
	//if (refEntity.m_upPlayerNickName)
	//{
	//	const auto& spacePlayer = refEntity.m_refSpace.GetSpacePlayer(refEntity);
	//	u16等级 = spacePlayer.单位属性等级(refEntity.m_类型, 防御);
	//}

	//return 单位::单位防御(refEntity.m_类型, u16等级);
	return EntitySystem::升级后防御(refEntity);
}

template<class T>
void SendToWorldSvr(const T& refMsg);

static void 记录击杀(const uint32_t uSvrId, const 战局类型 类型, const std::string strPlayerNickName攻击, const 单位类型 攻击, const std::string strPlayerNickName阵亡, const 单位类型 阵亡)
{
	if (!单位::战局配置::Is多人混战(类型))
		return;

	if (strPlayerNickName攻击.empty() || strPlayerNickName阵亡.empty())
		return;

	SendToWorldSvr(Msg击杀{ .uSvrId = uSvrId, .战局 = 类型, .nickName攻击 = strPlayerNickName攻击, .单位类型攻击 = 攻击, .nickName阵亡 = strPlayerNickName阵亡, .单位类型阵亡 = 阵亡 });
}


void DefenceComponent::受伤(const int 攻击, const Entity& refAttacker)
{
	CHECK_GE(攻击, 0);
	if (IsDead())
		return;

	const auto 防御 = 升级后的防御(m_refEntity);
	const auto 伤害 = std::max(攻击 - 防御, 1);

	数值Component::改变(m_refEntity, 生命, -伤害); //this->m_hp -= 伤害;
	m_map对我伤害[refAttacker.Id] += 伤害;

	播放正遭到攻击语音();
	if (IsDead())
	{
		{
			uint32_t GetSvrId();
			const uint32_t uSvrId = GetSvrId();
			记录击杀(uSvrId, m_refEntity.m_refSpace.m_配置.战局, EntitySystem::GetNickName(refAttacker), refAttacker.m_类型, EntitySystem::GetNickName(m_refEntity), m_refEntity.m_类型);
		}

		单位::单位配置 单位;
		if (单位::Find单位配置(m_refEntity.m_类型, 单位))
		{
			EntitySystem::Broadcast播放声音(m_refEntity, 单位.str阵亡音效);
			m_refEntity.BroadcastChangeSkeleAnim(单位.阵亡.str名字或索引, false, 单位.阵亡.f播放速度, 单位.阵亡.f起始时刻秒, 单位.阵亡.f结束时刻秒);//播放阵亡动作
		}
		else
		{
			LOG(WARNING) << "没有类型:" << m_refEntity.m_类型;
		}
		m_refEntity.DelayDelete();
		CoEvent<MyEvent::单位阵亡>::OnRecvEvent({ .wpEntity = m_refEntity.weak_from_this() });
		PlayerComponent::Send资源(m_refEntity);
		if (m_refEntity.m_up临时阻挡)
		{
			m_refEntity.m_up临时阻挡->OnEntityDestroy(false);
		}
	}
}


bool DefenceComponent::IsDead() const
{
	return 数值Component::Get(m_refEntity, 生命) <= 0;
}

void DefenceComponent::播放正遭到攻击语音()
{
	using namespace std;
	const auto now = std::chrono::system_clock::now();
	if (now - m_time上次提醒正遭到攻击 < 60s)
		return;

	m_time上次提醒正遭到攻击 = now;

	LPCSTR sz语音 = "";
	switch (m_refEntity.m_类型)
	{
	case 基地:sz语音 = "语音/基地正遭到进攻女声正经版"; break;
	case 工程车:sz语音 = "语音/工程车遭到攻击女声可爱版"; break;
	case 虫巢:sz语音 = "语音/虫巢正遭到攻击_男声"; break;
	case 工虫:sz语音 = "语音/工虫遭到攻击_男声"; break;
	default:
		return;
		break;
	}
	PlayerComponent::播放声音(m_refEntity, sz语音, m_refEntity.m_配置.strName + " 遭到攻击");
}

int DefenceComponent::最大生命()const
{
	return 数值Component::Get(m_refEntity, 属性类型::最大生命);
}

bool DefenceComponent::已满血() const
{
	return 最大生命() <= 数值Component::Get(m_refEntity, 生命);
}
