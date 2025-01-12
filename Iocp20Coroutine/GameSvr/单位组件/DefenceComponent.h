#pragma once
class Entity;
class DefenceComponent
{
public:
	DefenceComponent(Entity& refEntity, const int i32HpMax);
	static void AddComponent(Entity& refEntity, uint16_t u16初始Hp);
	void 受伤(int);
	bool IsDead() const;
	void 播放基地正遭到攻击语音();
	int m_hp = 20;
	const int32_t m_i32HpMax;

private:
	Entity& m_refEntity;
	std::chrono::system_clock::time_point m_time上次提醒基地正遭到进攻;
};

