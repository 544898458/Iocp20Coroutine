#pragma once

#include "../CoRoutine/CoTask.h"
#include "../SpEntity.h"

class ̦��Component
{
public:
	̦��Component(Entity&);
	CoTaskBool Co̦������();
	static void AddComponent(Entity& refEntity);
	void TryCancel();
	int16_t m_i16�뾶 = 5;
	/// <summary>
	/// ������Ž�����ʧ����������
	/// </summary>
	WpEntity m_wp���Ž���;

private:
	FunCancel m_funCancel;
	/// <summary>
	/// �Լ���̦��
	/// </summary>
	Entity& m_refEntity;
};