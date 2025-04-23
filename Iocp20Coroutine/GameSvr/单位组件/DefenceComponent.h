#pragma once
#include <map>
class Entity;
class DefenceComponent
{
public:
	DefenceComponent(Entity& refEntity, const uint16_t u16��ʼHp);
	static void AddComponent(Entity& refEntity, uint16_t u16��ʼHp);
	static uint16_t ������ķ���(Entity& refEntity);
	void ����(int hp, const uint64_t idAttacker);
	bool IsDead() const;
	void �������⵽��������();
	int �������()const;
	bool ����Ѫ()const;
	void ��Ѫ(int16_t i16�仯);
	std::map<uint64_t, int> m_map�����˺�;

private:
	Entity& m_refEntity;
	std::chrono::system_clock::time_point m_time�ϴ��������⵽����;
};

