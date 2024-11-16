#pragma once
class Entity;
class DefenceComponent
{
public:
	DefenceComponent(Entity& refEntity);
	static void AddComponent(Entity& refEntity, uint16_t u16≥ı ºHp);
	void  ‹…À(int);
	bool IsDead() const;
	int m_hp = 20;
private:
	Entity& m_refEntity;
};

