#pragma once

class Entity;

namespace EntitySystem
{
	class 恢复休闲动作 final
	{
	public:
		恢复休闲动作(Entity& refEntity, const std::string& str动作);
		~恢复休闲动作();
	private:
		Entity& m_refEntity;
	};
	void BroadcastEntity描述(Entity& refEntity, const std::string& refStrGbk);
	void BroadcastChangeSkeleAnimIdle(Entity& refEntity);
	void BroadcastChangeSkeleAnim采集(Entity& refEntity);
	void Broadcast播放声音(Entity& refEntity, const std::string& refStr声音, const std::string& str文本 = "");
	bool Is视口(const Entity& refEntity);
	bool Is光刺(const Entity& refEntity);
	bool 距离友方单位太近(Entity& refEntity);
	const std::string GetNickName(Entity& refEntity);
	bool Is建筑(const 单位类型 类型);
	bool Is可进地堡(const 单位类型 类型);
	bool Is资源(const 单位类型 类型);
	bool Is活动单位(const 单位类型 类型);
	bool Is单位类型(const WpEntity& wp, const 单位类型 类型);
	bool Is空地能打(const 单位类型 攻, const 单位类型 防);
	bool Is怪(const 单位类型 类型);
	bool Is视口(const 单位类型 类型);
};

