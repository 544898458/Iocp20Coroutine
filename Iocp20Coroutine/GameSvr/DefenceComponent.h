#pragma once
class Entity;
class DefenceComponent
{
public:
	DefenceComponent(Entity& refEntity, const int i32HpMax);
	static void AddComponent(Entity& refEntity, uint16_t u16≥ı ºHp);
	void  ‹…À(int);
	bool IsDead() const;
	int m_hp = 20;
	const int32_t m_i32HpMax;
private:
	Entity& m_refEntity;
};

