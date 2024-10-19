#include "pch.h"
#include "��ԴComponent.h"
#include "Space.h"
#include "MyMsgQueue.h"
#include "Entity.h"
#include "��ʱ�赲Component.h"
#include "��λ.h"

void ��ԴComponent::Add(Space& refSpace, const ��Դ���� ����)
{
	��λ::��Դ��λ���� ���� = {};
	if (!��λ::Find��Դ��λ����(����, ����))
	{
		LOG(ERROR) << "Add" << ����;
		assert(false);
		return;
	}

	SpEntity spEntity = std::make_shared<Entity, const Position&, Space&, const std::string&, const std::string& >(
		{ 20,float(rand() % 30) }, refSpace, ����.����.strPrefabName, ����.����.strName);
	refSpace.m_mapEntity.insert({ spEntity->Id ,spEntity });
	//LOG(INFO) << "SpawnMonster:" << refSpace.m_mapEntity.size();
	spEntity->m_sp��Դ = std::make_shared<��ԴComponent, const ��Դ����>(std::forward<const ��Դ����&&>(����));
	��ʱ�赲Component::AddComponent(*spEntity, 0.5f);
	spEntity->BroadcastEnter();
}