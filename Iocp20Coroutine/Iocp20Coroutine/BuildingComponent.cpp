#include "pch.h"
#include "BuildingComponent.h"
#include "Entity.h"
#include "PlayerComponent.h"
#include "PlayerGateSession_Game.h"
#include "��ʱ�赲Component.h"

void BuildingComponent::AddComponent(Entity& refThis, PlayerGateSession_Game& refSession, const ������λ���� ����, float f��߳�)
{
	refThis.m_spBuilding = std::make_shared<BuildingComponent, PlayerGateSession_Game&, const ������λ����&,Entity&>(refSession, ����,refThis);
	��ʱ�赲Component::AddComponent(refThis, f��߳�);
}

BuildingComponent::BuildingComponent(PlayerGateSession_Game& refSession, const ������λ����& ����, Entity &refEntity) :m_����(����), m_refEntity(refEntity)
{
	//if (!m_coAddMoney.Finished())
	//{
	//	LOG(INFO) << "ǰһ���콨��Э�̻�û����";
	//	return;
	//}
	//m_coAddMoney = AiCo::AddMoney(refSession, m_cancelAddMoney);
	//m_coAddMoney.Run();
	
}