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
	void OnEntityDestroy(const bool bDestroy);
	bool �ڰ뾶��(const Position& pos)const;
	bool ���Ž���() const;
	
	int16_t m_i16�뾶 = 5;
	/// <summary>
	/// ������Ž�����ʧ����������
	/// </summary>
	WpEntity m_wp���Ž���;

private:
	CoTaskBool Coή������();
	CoTaskBool Co����Χ��Buff();

	
	FunCancel��ȫ m_funCancelή������;
	FunCancel��ȫ m_funCancel����Χ��Buff;
	/// <summary>
	/// �Լ���̦��
	/// </summary>
	Entity& m_refEntity;
};