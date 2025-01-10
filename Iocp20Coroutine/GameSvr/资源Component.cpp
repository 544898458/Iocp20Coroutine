#include "pch.h"
#include "��ԴComponent.h"
#include "Space.h"
#include "MyMsgQueue.h"
#include "Entity.h"
#include "��ʱ�赲Component.h"
#include "��λ.h"

void ��ԴComponent::Add(Space& refSpace, const ��λ���� ����, const Position& refPosition)
{
	��λ::��Դ��λ���� ���� = {};
	if (!��λ::Find��Դ��λ����(����, ����))
	{
		LOG(ERROR) << "Add" << ����;
		assert(false);
		return;
	}

	SpEntity spEntity = std::make_shared<Entity, const Position&, Space&, const ��λ::��λ����&>(
		refPosition, refSpace, std::forward<const ��λ����&&>(����), ����.����);
	refSpace.AddEntity(spEntity);
	//LOG(INFO) << "SpawnMonster:" << refSpace.m_mapEntity.size();
	spEntity->m_sp��Դ = std::make_shared<��ԴComponent, const ��λ����>(std::forward<const ��λ����&&>(����));
	��ʱ�赲Component::AddComponent(*spEntity, 2.f);
	spEntity->BroadcastEnter();
}