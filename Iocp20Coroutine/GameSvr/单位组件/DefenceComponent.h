#pragma once
#include <map>
class Entity;
class DefenceComponent
{
public:
	DefenceComponent(Entity& refEntity, const int i32HpMax);
	static void AddComponent(Entity& refEntity, uint16_t u16初始Hp);
	static uint16_t 升级后的防御(Entity& refEntity);
	void 受伤(int hp, const uint64_t idAttacker);
	bool IsDead() const;
	void 播放正遭到攻击语音();
	bool 已满血()const;
	void 加血(int16_t i16变化);
	int m_hp = 20;
	const int32_t m_i32HpMax;
	std::map<uint64_t, int> m_map对我伤害;

private:
	Entity& m_refEntity;
	std::chrono::system_clock::time_point m_time上次提醒正遭到攻击;
};

