#include "pch.h"
#include "BuildingComponent.h"
#include "Entity.h"
#include "PlayerComponent.h"
#include "PlayerGateSession_Game.h"
#include "临时阻挡Component.h"

void BuildingComponent::AddComponent(Entity& refThis, PlayerGateSession_Game& refSession, const 建筑单位类型 类型, float f半边长)
{
	refThis.m_spBuilding = std::make_shared<BuildingComponent, PlayerGateSession_Game&, const 建筑单位类型&,Entity&>(refSession, 类型,refThis);
	临时阻挡Component::AddComponent(refThis, f半边长);
}

BuildingComponent::BuildingComponent(PlayerGateSession_Game& refSession, const 建筑单位类型& 类型, Entity &refEntity) :m_类型(类型), m_refEntity(refEntity)
{
	//if (!m_coAddMoney.Finished())
	//{
	//	LOG(INFO) << "前一个造建筑协程还没返回";
	//	return;
	//}
	//m_coAddMoney = AiCo::AddMoney(refSession, m_cancelAddMoney);
	//m_coAddMoney.Run();
	
}