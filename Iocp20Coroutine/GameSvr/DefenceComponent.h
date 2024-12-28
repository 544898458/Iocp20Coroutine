#pragma once
class Entity;
class DefenceComponent
{
public:
	DefenceComponent(Entity& refEntity, const int i32HpMax);
	static void AddComponent(Entity& refEntity, uint16_t u16��ʼHp);
	void ����(int);
	bool IsDead() const;
	void ���Ż������⵽��������();
	int m_hp = 20;
	const int32_t m_i32HpMax;

private:
	Entity& m_refEntity;
	std::chrono::system_clock::time_point m_time�ϴ����ѻ������⵽����;
};

