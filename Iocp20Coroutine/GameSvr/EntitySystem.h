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
	bool Is活动单位建筑怪(const Entity& refEntity);
	bool 距离友方单位太近(Entity& refEntity);
	const std::string GetNickName(const Entity& refEntity);
	bool Is建筑(const 单位类型 类型);
	bool Is建筑(const Entity& refEntity);
	bool Is可进地堡(const 单位类型 类型);
	bool Is资源(const 单位类型 类型);
	bool Is活动单位(const 单位类型 类型);
	bool Is单位类型(const WpEntity& wp, const 单位类型 类型);
	bool Is空地能打(const 单位类型 攻, const 单位类型 防);
	bool Is怪(const 单位类型 类型);
	bool Is视口(const 单位类型 类型);
	bool Is光刺(const 单位类型 类型);
	bool Is活动单位建筑怪(const 单位类型 类型);
	template<typename T_成员>
	T_成员 升级后属性(const Entity& refEntity, const 属性类型 属性, T_成员 单位::战斗配置::* p成员);
	uint16_t 升级后攻击(Entity& refEntity);
	uint16_t 升级后防御(Entity& refEntity);
	uint16_t 升级后攻击前摇_伤害耗时(Entity& refEntity);
	float 升级后速度每帧移动距离(Entity& refEntity);
	float 升级后速度每秒移动距离(Entity& refEntity);
	void 停止攻击和治疗(Entity& refEntity);
	void BroadcastEntity苔蔓半径(Entity& refEntity);
};

