#include "pch.h"
#include "Entity.h"
#include "EntitySystem.h"
#include "MyMsgQueue.h"
#include "../IocpNetwork/StrConv.h"

void EntitySystem::BroadcastEntity√Ë ˆ(Entity& refEntity, const std::string& refStrGbk)
{
	refEntity.Broadcast<MsgEntity√Ë ˆ>({ .idEntity = refEntity.Id, .str√Ë ˆ = StrConv::GbkToUtf8(refStrGbk) });
}

void EntitySystem::BroadcastChangeSkeleAnimIdle(Entity& refEntity)
{
	refEntity.BroadcastChangeSkeleAnim("idle");
}