#pragma once

#include "../CoRoutine/CoTask.h"
#include "../SpEntity.h"

struct Position;

class ̦��Component
{
public:
	static const uint16_t MAX�뾶 = 20;
	̦��Component(Entity&);
	static void AddComponent(Entity& refEntity);
	void TryCancel();
	bool �ڰ뾶��(const Position& pos)const;
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