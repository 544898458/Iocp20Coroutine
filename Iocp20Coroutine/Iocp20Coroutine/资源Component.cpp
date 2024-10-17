#include "pch.h"
#include "��ԴComponent.h"
#include "Space.h"
#include "MyMsgQueue.h"
#include "Entity.h"
#include "��ʱ�赲Component.h"
void ��ԴComponent::Add(Space& refSpace)
{
	SpEntity spEntity = std::make_shared<Entity, const Position&, Space&, const std::string&, const std::string& >({ 30.0 }, refSpace, "tree_large", "�����");
	refSpace.m_mapEntity.insert({ spEntity->Id ,spEntity});
	//LOG(INFO) << "SpawnMonster:" << refSpace.m_mapEntity.size();
	spEntity->m_sp��Դ = std::make_shared<��ԴComponent>();
	��ʱ�赲Component::AddComponent(*spEntity, 1);
	spEntity->BroadcastEnter();
}