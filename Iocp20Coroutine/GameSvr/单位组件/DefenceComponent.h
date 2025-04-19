#pragma once
#include <map>
class Entity;
class DefenceComponent
{
public:
	DefenceComponent(Entity& refEntity, const int i32HpMax);
	static void AddComponent(Entity& refEntity, uint16_t u16��ʼHp);
	static uint16_t ������ķ���(Entity& refEntity);
	void ����(int hp, const uint64_t idAttacker);
	bool IsDead() const;
	void �������⵽��������();
	bool ����Ѫ()const;
	void ��Ѫ(int16_t i16�仯);
	int m_hp = 20;
	const int32_t m_i32HpMax;
	std::map<uint64_t, int> m_map�����˺�;

private:
	Entity& m_refEntity;
	std::chrono::system_clock::time_point m_time�ϴ��������⵽����;
};

