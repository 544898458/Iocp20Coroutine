#include "pch.h"
#include "Entity.h"
#include "EntitySystem.h"
#include "MyMsgQueue.h"
#include "../IocpNetwork/StrConv.h"

void EntitySystem::BroadcastEntity����(Entity& refEntity, const std::string& refStrGbk)
{
	refEntity.Broadcast<MsgEntity����>({ .idEntity = refEntity.Id, .str���� = StrConv::GbkToUtf8(refStrGbk) });
}

void EntitySystem::BroadcastChangeSkeleAnimIdle(Entity& refEntity)
{
	refEntity.BroadcastChangeSkeleAnim("idle");
}