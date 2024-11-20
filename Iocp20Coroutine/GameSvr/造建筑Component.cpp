#include "pch.h"
#include "造建筑Component.h"
#include "单位.h"
#include "AiCo.h"
#include "Entity.h"
#include "PlayerComponent.h"
#include "PlayerGateSession_Game.h"

void 造建筑Component::AddComponet(Entity& refEntity, PlayerGateSession_Game& refGateSession, const 活动单位类型 类型)
{
}

造建筑Component::造建筑Component(PlayerGateSession_Game& refSession, Entity& refEntity, const 活动单位类型 类型):m_refEntity(refEntity)
{
	switch (类型)
	{
	case 工程车:
		m_set可造类型.insert(基地); 
		m_set可造类型.insert(民房);
		m_set可造类型.insert(兵厂);
		m_set可造类型.insert(地堡);
		break;
	default:
		break;
	}
}

CoTaskBool 造建筑Component::Co造建筑(const Position &refPos, const 建筑单位类型 类型)
{
	//先走到目标点
	if (co_await AiCo::WalkToPos(m_refEntity, refPos, m_cancel造建筑))
		co_return true;

	//然后开始扣钱建造
	co_await m_refEntity.m_spPlayer->m_refSession.CoAddBuilding(类型, refPos);
	co_return true;
}
