#pragma once

#include "../SpEntity.h"

struct Position;
class ̫��Component
{
public:
	̫��Component(Entity&);
	static void AddComponent(Entity& refEntity);
	void ����(const Position& refPos);
	WpEntity m_wp����;
private:

	/// <summary>
	/// �Լ��ǽ������泲����̫��
	/// </summary>
	Entity& m_refEntity;
};

