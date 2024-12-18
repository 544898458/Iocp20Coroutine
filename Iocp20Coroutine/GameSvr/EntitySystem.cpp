#include "pch.h"
#include "Entity.h"
#include "EntitySystem.h"
#include "MyMsgQueue.h"
#include "../IocpNetwork/StrConv.h"

void EntitySystem::BroadcastEntityÃèÊö(Entity& refEntity, const std::string& refStrGbk)
{
	refEntity.Broadcast<MsgEntityÃèÊö>({ .idEntity = refEntity.Id, .strÃèÊö = StrConv::GbkToUtf8(refStrGbk) });
}

void EntitySystem::BroadcastChangeSkeleAnimIdle(Entity& refEntity)
{
	refEntity.BroadcastChangeSkeleAnim("idle");
}

void EntitySystem::Broadcast²¥·ÅÉùÒô(Entity& refEntity, const std::string& refStrÉùÒô, const std::string& strÎÄ±¾)
{
	refEntity.Broadcast<Msg²¥·ÅÉùÒô>({ .strÉùÒô = StrConv::GbkToUtf8(refStrÉùÒô), .strÎÄ±¾ = StrConv::GbkToUtf8(strÎÄ±¾) });
}
