#include "pch.h"
#include "资源Component.h"
#include "Space.h"
#include "MyMsgQueue.h"
#include "Entity.h"
#include "临时阻挡Component.h"
void 资源Component::Add(Space& refSpace)
{
	SpEntity spEntity = std::make_shared<Entity, const Position&, Space&, const std::string&, const std::string& >({ 30.0 }, refSpace, "tree_large", "晶体矿");
	refSpace.m_mapEntity.insert({ spEntity->Id ,spEntity});
	//LOG(INFO) << "SpawnMonster:" << refSpace.m_mapEntity.size();
	spEntity->m_sp资源 = std::make_shared<资源Component>();
	临时阻挡Component::AddComponent(*spEntity, 1);
	spEntity->BroadcastEnter();
}