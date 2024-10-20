#include "pch.h"
#include "�ر�Component.h"
#include "Entity.h"
#include "Space.h"

void �ر�Component::AddComponet(Entity& refEntity, PlayerGateSession_Game& refGateSession)
{
	refEntity.m_sp�ر� = std::make_shared<�ر�Component>();
}

void �ر�Component::��(Space& refSpace, uint64_t idEntity)
{
	//�ӵ�ͼ��ɾ������¼�ڵر���
	auto wp = refSpace.GetEntity(idEntity);
	CHECK_RET_VOID(!wp.expired());
	auto sp = wp.lock();
	m_vecSpEntity.push_back(sp);
	refSpace.m_mapEntity.erase(idEntity);
	sp->BroadcastLeave();
}
