#pragma once
#include "../CoRoutine/CoTask.h"
#include "../SpEntity.h"

class ��̦���ͳ�����ѪComponent
{
public:
	��̦���ͳ�����ѪComponent(Entity&);
	CoTaskBool Co�����ӳ�����ѪBuff();
	void OnEntityDestroy(const bool bDestroy);
	static void AddComponent(Entity& refEntity);
	WpEntity m_wp̦��;

private:
	FunCancel m_funCancel;
	/// <summary>
	/// �Լ��ǽ������泲����̫��
	/// </summary>
	Entity& m_refEntity;
};

