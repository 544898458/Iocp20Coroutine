#include "pch.h"
#include "造建筑Component.h"
#include "单位.h"

void 造建筑Component::AddComponet(Entity& refEntity, PlayerGateSession_Game& refGateSession, const 活动单位类型 类型)
{
}

造建筑Component::造建筑Component(PlayerGateSession_Game& refSession, Entity& refEntity, const 活动单位类型 类型)
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
