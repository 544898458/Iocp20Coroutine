#pragma once

class Entity;

namespace EntitySystem
{
	void BroadcastEntity����(Entity& refEntity, const std::string& refStrGbk);
	void BroadcastChangeSkeleAnimIdle(Entity& refEntity);
};

