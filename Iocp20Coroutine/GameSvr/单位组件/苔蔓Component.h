#pragma once

#include "../CoRoutine/CoTask.h"
#include "../SpEntity.h"

class ̦��Component
{
public:
	̦��Component(Entity&);
	static void AddComponent(Entity& refEntity);
	void TryCancel();
	int16_t m_i16�뾶 = 5;
	/// <summary>
	/// ������Ž�����ʧ����������
	/// </summary>
	WpEntity m_wp���Ž���;

private:
	CoTaskBool Coή������();
	CoTaskBool Co����Χ��Buff();

	FunCancel m_funCancelή������;
	FunCancel m_funCancel����Χ��Buff;
	/// <summary>
	/// �Լ���̦��
	/// </summary>
	Entity& m_refEntity;
};