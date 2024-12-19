#include "pch.h"
#include "Entity.h"
#include "EntitySystem.h"
#include "MyMsgQueue.h"
#include "../IocpNetwork/StrConv.h"

void EntitySystem::BroadcastEntity描述(Entity& refEntity, const std::string& refStrGbk)
{
	refEntity.Broadcast<MsgEntity描述>({ .idEntity = refEntity.Id, .str描述 = StrConv::GbkToUtf8(refStrGbk) });
}

void EntitySystem::BroadcastChangeSkeleAnimIdle(Entity& refEntity)
{
	refEntity.BroadcastChangeSkeleAnim("idle");
}

void EntitySystem::Broadcast播放声音(Entity& refEntity, const std::string& refStr声音, const std::string& str文本)
{
	refEntity.Broadcast<Msg播放声音>({ .str声音 = StrConv::GbkToUtf8(refStr声音), .str文本 = StrConv::GbkToUtf8(str文本) });
}

bool EntitySystem::Is视口(const Entity& refEntity)
{
	if (refEntity.m_spDefence)
		return false;
	
	if (!refEntity.m_spPlayer)
		return false;

	return true;
}
