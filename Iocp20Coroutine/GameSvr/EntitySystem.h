#pragma once

class Entity;

namespace EntitySystem
{
	void BroadcastEntity描述(Entity& refEntity, const std::string& refStrGbk);
	void BroadcastChangeSkeleAnimIdle(Entity& refEntity);
	void Broadcast播放声音(Entity& refEntity, const std::string& refStr声音, const std::string& str文本 = "");
	bool Is视口(const Entity& refEntity);
	bool 距离友方单位太近(Entity& refEntity);
};

