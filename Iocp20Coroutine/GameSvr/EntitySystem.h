#pragma once

class Entity;

namespace EntitySystem
{
	void BroadcastEntityÃèÊö(Entity& refEntity, const std::string& refStrGbk);
	void BroadcastChangeSkeleAnimIdle(Entity& refEntity);
	void Broadcast²¥·ÅÉùÒô(Entity& refEntity, const std::string& refStrÉùÒô);
};

