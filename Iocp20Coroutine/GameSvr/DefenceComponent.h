#pragma once
class Entity;
class DefenceComponent
{
public:
	DefenceComponent(Entity& refEntity);
	static void AddComponent(Entity& refEntity, uint16_t u16��ʼHp);
	void ����(int);
	bool IsDead() const;
	int m_hp = 20;
private:
	Entity& m_refEntity;
};

